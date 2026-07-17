//
// Created by Thomas Ibanez on 19.11.20.
//

#include "RenderSystem.h"

#include <SkeletonPoseComponent.h>
#include <SkinningComponent.h>

#include "Registry.h"

namespace ICE {
namespace {

// A fully-resolved renderable: everything needed to frustum-cull and assemble a Drawable, gathered
// on the render thread (Phase 1) so the parallel phase (Phase 2) touches no registry, asset bank or
// GPU bank -- only plain data and stable pointers. mesh/material/shader/textures are owning handles;
// skinning/pose are non-owning pointers into the mesh asset and the pose component, both of which
// outlive the frame and (after P4) have stable addresses.
struct RenderJob {
    Eigen::Vector3f worldCenter;
    Eigen::Vector3f worldExtents;
    Eigen::Matrix4f model_matrix;
    MeshHandle mesh;
    std::shared_ptr<Material> material;
    ShaderHandle shader;
    AssetUID material_uid = NO_ASSET_ID;
    const SkinningData *skinning = nullptr;
    const SkeletonPoseComponent *pose = nullptr;
};

// Phase 1 (render thread): read components, refresh the world-space bounds cache, resolve GPU
// resources (which may lazily upload -- hence render-thread only), and gather skinning inputs.
// Returns false (job discarded) if the mesh/material/shader can't be resolved.
bool resolveJob(Registry *reg, GPURegistry *gpu, std::unordered_map<Entity, CullingData> &cache, Entity e, RenderJob &job) {
    auto tc = reg->getComponent<TransformComponent>(e);
    auto rc = reg->getComponent<RenderComponent>(e);

    // Resolve GPU resources first (uploading on first use). meshHandle() returns an invalid handle
    // when the mesh asset is gone -- removed or evicted, e.g. after a re-import -- so this both gates
    // the job and guarantees the mesh asset is present before getMeshAABB() dereferences it below.
    // The material stays a CPU-asset shared_ptr; its textures are resolved by the geometry pass.
    auto mesh = gpu->meshHandle(rc->mesh);
    auto material = gpu->getMaterial(rc->material);
    if (!mesh.valid() || !material) {
        return false;
    }
    auto shader = gpu->shaderHandle(material->getShader());
    if (!shader.valid()) {
        return false;
    }

    Eigen::Matrix4f model_mat = tc->getWorldMatrix();

    // World-space bounds, recomputed only when the transform or mesh changes (single hash lookup).
    auto cache_it = cache.find(e);
    if (cache_it == cache.end() || cache_it->second.lastTransformVersion != tc->getVersion() || cache_it->second.lastMesh != rc->mesh) {
        auto local_aabb = gpu->getMeshAABB(rc->mesh);
        Eigen::Vector3f localCenter = local_aabb.getCenter();
        Eigen::Vector3f localExtents = local_aabb.getExtent();

        Eigen::Matrix3f R = model_mat.block<3, 3>(0, 0);
        Eigen::Vector3f T = model_mat.block<3, 1>(0, 3);
        Eigen::Vector3f worldCenter = R * localCenter + T;
        Eigen::Matrix3f absR = R.cwiseAbs();
        Eigen::Vector3f worldExtents = absR * localExtents;

        CullingData data{
            .lastTransformVersion = tc->getVersion(),
            .lastMesh = rc->mesh,
            .worldCenter = worldCenter,
            .worldExtents = worldExtents,
        };
        if (cache_it == cache.end()) {
            cache_it = cache.emplace(e, data).first;
        } else {
            cache_it->second = data;
        }
    }
    job.worldCenter = cache_it->second.worldCenter;
    job.worldExtents = cache_it->second.worldExtents;

    if (reg->entityHasComponent<SkinningComponent>(e)) {
        auto skeleton_entity = reg->getComponent<SkinningComponent>(e)->skeleton_entity;
        // The skeleton entity may be stale/invalid; probe instead of asserting so a bad reference
        // skips skinning for this frame rather than dereferencing null.
        auto pose = reg->tryGetComponent<SkeletonPoseComponent>(skeleton_entity);
        auto skel_transform = reg->tryGetComponent<TransformComponent>(skeleton_entity);
        if (pose && skel_transform) {
            job.skinning = &gpu->getMeshSkinningData(rc->mesh);
            job.pose = pose;
            model_mat = skel_transform->getWorldMatrix();
        }
    }

    job.model_matrix = model_mat;
    job.mesh = mesh;
    job.material = std::move(material);
    job.shader = shader;
    job.material_uid = rc->material;
    return true;
}

// Phase 2 (any thread): frustum-cull, compute skinning bone matrices, and assemble the Drawable.
// Pure computation over the job's own data -- no shared mutable state -- so disjoint jobs run
// concurrently. Consumes `job` (moved-from) since each job is processed exactly once.
template<typename Frustum>
bool cullAndAssemble(RenderJob &job, const Frustum &frustum, Drawable &out) {
    if (!isAABBInFrustum(frustum, job.worldCenter, job.worldExtents)) {
        return false;
    }
    std::unordered_map<int, Eigen::Matrix4f> bone_matrices;
    if (job.skinning && job.pose) {
        for (const auto &[id, ibm] : job.skinning->inverseBindMatrices) {
            // bone_transform is indexed by bone id; guard against an id outside the current pose.
            if (id >= 0 && static_cast<size_t>(id) < job.pose->bone_transform.size()) {
                bone_matrices.try_emplace(id, job.pose->bone_transform[id] * ibm);
            }
        }
    }
    out = Drawable{
        .mesh = job.mesh,
        .material = std::move(job.material),
        .shader = job.shader,
        .material_uid = job.material_uid,
        .model_matrix = job.model_matrix,
        .bone_matrices = std::move(bone_matrices),
    };
    return true;
}

}  // namespace

RenderSystem::RenderSystem(const std::shared_ptr<Registry> &reg, const std::shared_ptr<GPURegistry> &gpu_bank)
    : m_registry(reg.get()),
      m_gpu_bank(gpu_bank) {
}

void RenderSystem::update(double delta) {

    auto view_mat = m_camera->lookThrough();
    auto proj_mat = m_camera->getProjection();

    if (m_skybox != NO_ASSET_ID) {
        m_renderer->submitSkybox(Skybox{
            .cube_mesh = m_gpu_bank->meshHandle(AssetPath::WithTypePrefix<Mesh>("cube")),
            .shader = m_gpu_bank->shaderHandle(AssetPath::WithTypePrefix<Shader>("__ice_skybox_shader")),
        });
    }

    auto frustum = extractFrustumPlanes(proj_mat * view_mat);

    // Phase 1 (render thread): resolve every renderable into a self-contained job. All component,
    // asset-bank and GPU-bank access -- including lazy GL uploads -- happens here, single-threaded.
    std::vector<RenderJob> jobs;
    jobs.reserve(m_render_queue.size());
    for (const auto &e : m_render_queue) {
        RenderJob job;
        if (resolveJob(m_registry, m_gpu_bank.get(), m_culling_cache, e, job)) {
            jobs.push_back(std::move(job));
        }
    }

    // Phase 2: frustum-cull, skin and assemble a Drawable per surviving job. This is pure
    // computation over the pre-resolved data with each index independent (disjoint writes to
    // `drawables`/`visible`), so it runs across the scheduler's workers when one is set and the
    // batch is large enough; otherwise it runs inline. `visible` is char (not vector<bool>) so
    // concurrent writes to distinct elements are race-free.
    std::vector<Drawable> drawables(jobs.size());
    std::vector<char> visible(jobs.size(), 0);
    auto process = [&](size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i) {
            if (cullAndAssemble(jobs[i], frustum, drawables[i])) {
                visible[i] = 1;
            }
        }
    };
    static constexpr size_t kParallelThreshold = 256;
    if (m_scheduler && jobs.size() >= kParallelThreshold) {
        m_scheduler->parallelRanges(jobs.size(), [&](size_t begin, size_t end) { process(begin, end); });
    } else {
        process(0, jobs.size());
    }

    // Phase 3 (render thread): submit the survivors in queue order (the renderer sorts them anyway).
    for (size_t i = 0; i < jobs.size(); ++i) {
        if (visible[i]) {
            m_renderer->submitDrawable(std::move(drawables[i]));
        }
    }

    for (int i = 0; i < m_lights.size(); i++) {
        if (i >= MAX_LIGHTS)
            break;
        auto light = m_lights[i];
        auto lc = m_registry->getComponent<LightComponent>(light);
        auto tc = m_registry->getComponent<TransformComponent>(light);

        m_renderer->submitLight(Light{.position = tc->getPosition(),
                                      .rotation = tc->getRotationEulerDeg(),
                                      .color = lc->color,
                                      .distance_dropoff = lc->distance_dropoff,
                                      .type = lc->type});
    }

    // Hand the present target + shader to the renderer before render(): present is now a graph pass
    // (T10), so the whole frame -- geometry, features, UI, and the final composite -- runs inside
    // render(). m_target (nullptr = default framebuffer) is honoured by the present pass, preserving
    // the editor's render-to-texture path. Resolve the full-screen shader once and reuse it.
    if (!m_lastpass_shader) {
        m_lastpass_shader = m_gpu_bank->getShader(AssetPath::WithTypePrefix<Shader>("lastpass"));
    }
    m_renderer->setPresentTarget(m_target);
    m_renderer->setPresentShader(m_lastpass_shader);

    m_renderer->prepareFrame(*m_camera);
    m_renderer->render();
    m_renderer->endFrame();
}

void RenderSystem::onEntityAdded(Entity e) {
    // Resync from scratch: onEntityAdded fires on every signature change while the entity
    // matches, so clear this entity from all sub-lists first, then re-add based on its
    // current components. This keeps the renderables/lights/skybox lists correct when an
    // entity gains a second relevant component (e.g. a light added to a renderable).
    onEntityRemoved(e);
    if (m_registry->entityHasComponent<RenderComponent>(e)) {
        m_render_queue.emplace_back(e);
    }
    if (m_registry->entityHasComponent<LightComponent>(e)) {
        m_lights.emplace_back(e);
    }
    if (m_registry->entityHasComponent<SkyboxComponent>(e)) {
        m_skybox = e;
    }
}

void RenderSystem::onEntityRemoved(Entity e) {
    auto queue_pos = std::ranges::find(m_render_queue, e);
    if (queue_pos != m_render_queue.end()) {
        m_render_queue.erase(queue_pos);
    }
    auto light_pos = std::ranges::find(m_lights, e);
    if (light_pos != m_lights.end()) {
        m_lights.erase(light_pos);
    }
    if (e == m_skybox) {
        m_skybox = NO_ASSET_ID;
    }
    // Evict cached culling data so a recycled entity id can't inherit a stale AABB.
    m_culling_cache.erase(e);
}

std::shared_ptr<Renderer> RenderSystem::getRenderer() const {
    return m_renderer;
}

void RenderSystem::setRenderer(const std::shared_ptr<Renderer> &renderer) {
    m_renderer = renderer;
}

std::shared_ptr<Camera> RenderSystem::getCamera() const {
    return m_camera;
}

void RenderSystem::setCamera(const std::shared_ptr<Camera> &camera) {
    m_camera = camera;
}

void RenderSystem::setTarget(const std::shared_ptr<Framebuffer> &fb) {
    m_target = fb;
}

void RenderSystem::setViewport(int x, int y, int w, int h) {
    if (w > 0 && h > 0) {
        if (m_target)
            m_target->resize(w, h);
        m_renderer->resize(w, h);
    }
}

}  // namespace ICE