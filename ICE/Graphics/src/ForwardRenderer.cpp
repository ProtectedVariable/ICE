//
// Created by Thomas Ibanez on 20.11.20.
//

#include "ForwardRenderer.h"

#include <GL/gl3w.h>
#include <ICEMath.h>

#include <algorithm>
#include <LightComponent.h>
#include <Logger.h>
#include <Profiler.h>
#include <RenderComponent.h>
#include <RenderData.h>
#include <TransformComponent.h>

#include <unordered_set>

namespace ICE {

ForwardRenderer::ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory,
                                 const std::shared_ptr<GPURegistry>& gpu_registry)
    : m_api(api),
      m_gpu_registry(gpu_registry),
      m_geometry_pass(api, factory, gpu_registry, {1, 1, 1}),
      m_graph(factory) {

    m_camera_ubo = factory->createUniformBuffer(sizeof(CameraUBO), 0);
    m_light_ubo = factory->createUniformBuffer(sizeof(SceneLightsUBO), 1);

    // Full-screen quad for the present pass (moved here from RenderSystem, which no longer owns
    // any GL objects).
    m_present_quad = factory->createVertexArray();
    auto quad_vertex_vbo = factory->createVertexBuffer();
    quad_vertex_vbo->putData(full_quad_v.data(), full_quad_v.size() * sizeof(float));
    m_present_quad->pushVertexBuffer(quad_vertex_vbo, 3);
    auto quad_uv_vbo = factory->createVertexBuffer();
    quad_uv_vbo->putData(full_quad_tx.data(), full_quad_tx.size() * sizeof(float));
    m_present_quad->pushVertexBuffer(quad_uv_vbo, 2);
    auto quad_ibo = factory->createIndexBuffer();
    quad_ibo->putData(full_quad_idx.data(), full_quad_idx.size() * sizeof(int));
    m_present_quad->setIndexBuffer(quad_ibo);
}

void ForwardRenderer::submitSkybox(const Skybox& e) {
    m_skybox.emplace(e);
}
void ForwardRenderer::submitDrawable(Drawable e) {
    m_drawables.push_back(std::move(e));
}
void ForwardRenderer::submitLight(const Light& e) {
    m_lights.push_back(e);
}

void ForwardRenderer::uploadCameraUBO(Camera& camera) {
    auto view_mat = camera.lookThrough();
    auto proj_mat = camera.getProjection();
    auto cam_pos = camera.getPosition();

    CameraUBO camera_ubo_data{
        .projection = proj_mat, .view = view_mat, .cameraPos = Eigen::Vector4f(cam_pos.x(), cam_pos.y(), cam_pos.z(), 1.0f)};
    m_camera_ubo->putData(&camera_ubo_data, sizeof(CameraUBO));
}

void ForwardRenderer::prepareFrame(Camera& camera) {
    // Remembered for the frame so drawScene() can restore this view after a pass draws from
    // another one.
    m_frame_camera = &camera;
    uploadCameraUBO(camera);

    SceneLightsUBO light_ubo_data;
    // Clamp to the UBO's fixed capacity: lights[] is MAX_LIGHTS long, so writing more
    // would overflow the stack-allocated struct.
    const size_t light_count = std::min(m_lights.size(), static_cast<size_t>(MAX_LIGHTS));
    light_ubo_data.light_count = static_cast<int>(light_count);
    light_ubo_data.ambient_light = Eigen::Vector4f(0.1f, 0.1f, 0.1f, 1.0f);
    for (size_t i = 0; i < light_count; i++) {
        auto light = m_lights[i];
        light_ubo_data.lights[i].position = light.position;
        light_ubo_data.lights[i].rotation = light.rotation;
        light_ubo_data.lights[i].color = light.color;
        light_ubo_data.lights[i].distance_dropoff = light.distance_dropoff;
        light_ubo_data.lights[i].type = static_cast<int>(light.type);
    }
    m_light_ubo->putData(&light_ubo_data, sizeof(SceneLightsUBO));

    if (m_skybox.has_value()) {
        GPUMesh* sky_mesh = m_gpu_registry->resolve(m_skybox->cube_mesh);
        ShaderProgram* sky_shader = m_gpu_registry->resolve(m_skybox->shader);
        if (sky_mesh && sky_shader) {
            RenderCommand skybox_cmd;
            skybox_cmd.mesh = sky_mesh;
            skybox_cmd.material = nullptr;
            skybox_cmd.shader = sky_shader;
            skybox_cmd.model_matrix = Eigen::Matrix4f::Identity();
            skybox_cmd.is_instanced = false;
            // Draw after opaque geometry (so it only fills background pixels) but before
            // transparent. Its fragments sit at the far plane (z=w in skybox.vs), so it needs
            // GL_LEQUAL and must not write depth.
            skybox_cmd.depthTest = true;
            skybox_cmd.depthWrite = false;
            skybox_cmd.depth_func = DepthFunc::LEqual;
            skybox_cmd.sort_key = 0x7FFFFFFFFFFFFFFFULL;  // last among opaque (transparent bit 63 = 0)
            m_render_commands.push_back(skybox_cmd);
        }
    }

    // Instance batching: group drawables by the exact (mesh, material, shader) triple. Each
    // drawable's mesh/shader handle is resolved to a raw pointer once here, and those resolved
    // pointers ARE the batch key -- so the render commands and sort keys are unchanged.
    std::map<BatchKey, std::vector<const Drawable*>> instance_batches;
    std::vector<const Drawable*> non_instanced_drawables;  // Skinned meshes, etc.

    for (const auto& drawable : m_drawables) {
        GPUMesh* mesh = m_gpu_registry->resolve(drawable.mesh);
        ShaderProgram* shader = m_gpu_registry->resolve(drawable.shader);
        if (!mesh || !shader || !drawable.material) {
            continue;  // stale handle or missing material
        }
        // Skip instancing for skinned meshes (has bones)
        if (!drawable.bone_matrices.empty()) {
            non_instanced_drawables.push_back(&drawable);
            continue;
        }
        instance_batches[BatchKey{mesh, drawable.material.get(), shader}].push_back(&drawable);
    }

    // Convert batches to render commands
    m_instance_batches.clear();  // Clear previous frame's instance data
    Eigen::Vector3f camera_pos = camera.getPosition();

    for (const auto& [key, batch] : instance_batches) {
        GPUMesh* mesh = std::get<0>(key);
        Material* material = std::get<1>(key);
        ShaderProgram* shader = std::get<2>(key);
        if (batch.size() == 1) {
            // Single instance - use regular rendering
            const auto* drawable = batch[0];
            auto dist = (drawable->model_matrix.block<3, 1>(0, 3) - camera_pos).squaredNorm();
            RenderCommand cmd;
            cmd.mesh = mesh;
            cmd.material = material;
            cmd.shader = shader;
            cmd.model_matrix = drawable->model_matrix;
            cmd.depthTest = true;
            cmd.faceCulling = true;
            cmd.is_instanced = false;
            cmd.blend = material->isTransparent();
            cmd.computeSortKey(material->isTransparent(), dist, material->getShader(), drawable->material_uid);
            m_render_commands.push_back(cmd);
        } else {
            // Multiple instances - use instanced rendering
            // Store instance data in member variable for lifetime management
            auto& instance_data_vec = m_instance_batches[key];
            instance_data_vec.clear();
            instance_data_vec.reserve(batch.size());

            for (const auto* drawable : batch) {
                InstanceData inst_data;
                inst_data.model_matrix = drawable->model_matrix;
                instance_data_vec.push_back(inst_data);
            }

            const auto* first = batch[0];
            auto dist = (first->model_matrix.block<3, 1>(0, 3) - camera_pos).squaredNorm();
            RenderCommand cmd;
            cmd.mesh = mesh;
            cmd.material = material;
            cmd.shader = shader;
            cmd.depthTest = true;
            cmd.faceCulling = true;
            cmd.is_instanced = true;
            cmd.instance_count = batch.size();
            cmd.instance_data = &instance_data_vec;  // Link to stored data
            cmd.blend = material->isTransparent();
            cmd.computeSortKey(material->isTransparent(), dist, material->getShader(), first->material_uid);
            m_render_commands.push_back(cmd);
        }
    }

    // Add non-instanced drawables (skinned meshes)
    for (const auto* drawable : non_instanced_drawables) {
        GPUMesh* mesh = m_gpu_registry->resolve(drawable->mesh);
        ShaderProgram* shader = m_gpu_registry->resolve(drawable->shader);
        Material* material = drawable->material.get();
        RenderCommand cmd;
        auto dist = (drawable->model_matrix.block<3, 1>(0, 3) - camera_pos).squaredNorm();
        cmd.mesh = mesh;
        cmd.material = material;
        cmd.shader = shader;
        cmd.model_matrix = drawable->model_matrix;
        cmd.depthTest = true;
        cmd.faceCulling = true;
        cmd.bones = &drawable->bone_matrices;
        cmd.is_instanced = false;
        cmd.blend = material->isTransparent();
        cmd.computeSortKey(material->isTransparent(), dist, material->getShader(), drawable->material_uid);
        m_render_commands.push_back(cmd);
    }

    std::sort(m_render_commands.begin(), m_render_commands.end());

    m_geometry_pass.submit(&m_render_commands);
}

void ForwardRenderer::addPass(std::unique_ptr<IRenderPass> pass) {
    // Wrapped rather than kept in a second list, so passes and features share one registration
    // order and one wiring path in rebuildGraph().
    if (pass) {
        addFeature(std::make_unique<SinglePassFeature>(std::move(pass)));
    }
}

void ForwardRenderer::addFeature(std::unique_ptr<RenderFeature> feature) {
    if (feature) {
        m_features.push_back(std::move(feature));
        m_graph_dirty = true;  // the new feature's passes only join the graph on a rebuild
    }
}

void ForwardRenderer::drawScene(Camera& camera, ShaderProgram* override_shader) {
    // Point the shared camera UBO at the requested view, replay the frame's visible set into
    // whatever target the graph bound, then put the frame's own camera back. Without the restore, a
    // shadow pass drawing from a light would leave the geometry pass rendering from that light.
    uploadCameraUBO(camera);
    m_geometry_pass.drawInto(override_shader);
    if (m_frame_camera && m_frame_camera != &camera) {
        uploadCameraUBO(*m_frame_camera);
    }
}

void ForwardRenderer::fullscreen(ShaderProgram* shader) {
    if (!shader) {
        return;
    }
    shader->bind();
    m_present_quad->bind();
    m_present_quad->getIndexBuffer()->bind();
    m_api->renderVertexArray(m_present_quad);
}

void ForwardRenderer::rebuildGraph() {
    m_graph.reset();

    // Publish the geometry pass's framebuffer into the resource table so features can name the
    // scene colour with a typed handle. The geometry pass itself still declares its target
    // through the string layer (removed in T10).
    auto scene_color = m_graph.importResource<Framebuffer>("scene_color", m_geometry_pass.getResult());

    auto& geometry = m_graph.addPass("geometry");
    geometry.write("scene_color");
    geometry.setExecuteCallback([this](const RenderGraphPass&) {
        m_api->beginGPUTimer();
        m_geometry_pass.execute();
        Profiler::get().addSample("GPU::geometry", m_api->endGPUTimer());
    });

    // Application-registered features contribute their passes here -- the whole point of the seam:
    // no engine edit is needed to add a pass. setup() runs on each rebuild, which is what
    // regenerates every pass's resource handles for this compile.
    for (const auto& feature : m_features) {
        addFeaturePasses(m_graph, *feature, scene_color, m_api, this);
    }

    // Present is a graph pass too (T10): it reads the finished scene colour -- so it is ordered
    // after geometry and every feature/UI pass that wrote it -- and composites it to the frame's
    // present target. It writes a virtual "backbuffer" resource that is the graph's declared output,
    // which is what keeps it (and its dependencies) from being culled. The backbuffer has no
    // physical resource: the pass binds the real target (default framebuffer or the editor's RTT)
    // itself, since that isn't a graph-managed framebuffer.
    auto& present = m_graph.addPass("present");
    present.read("scene_color");
    present.write("backbuffer");
    present.setExecuteCallback([this, scene_color](const RenderGraphPass&) {
        auto* resource = m_graph.resourceAt(scene_color.index());
        auto scene_fb = resource ? resource->getPhysicalResourceAs<Framebuffer>() : nullptr;
        if (!m_present_shader || !scene_fb) {
            return;
        }
        // Off-screen (editor viewport) if a target is set, else the window's default framebuffer.
        if (m_present_target) {
            m_present_target->bind();
            m_api->setViewport(0, 0, static_cast<int>(m_present_target->getFormat().width), static_cast<int>(m_present_target->getFormat().height));
        } else {
            m_api->bindDefaultFramebuffer();
            m_api->setViewport(0, 0, static_cast<int>(scene_fb->getFormat().width), static_cast<int>(scene_fb->getFormat().height));
        }
        m_api->clear();
        m_present_shader->bind();
        scene_fb->bindAttachment(0);
        m_present_shader->loadInt("uTexture", 0);
        m_present_quad->bind();
        m_present_quad->getIndexBuffer()->bind();
        m_api->renderVertexArray(m_present_quad);
    });

    m_graph.setOutput("backbuffer");
    m_graph.compile();
}

std::shared_ptr<Framebuffer> ForwardRenderer::render() {
    // The graph owns the whole frame -- geometry, feature/UI passes, and present. It is compiled
    // once and re-executed each frame; a rebuild happens only when its shape changes (a resize, or
    // a newly registered pass/feature). The present pass composites to the frame's target, so
    // render() has no separate present step. Returns the scene colour for callers that read it
    // (e.g. the editor's picking pass).
    if (m_graph_dirty) {
        rebuildGraph();
        m_graph_dirty = false;
    }
    m_graph.execute();
    m_output_fb = m_geometry_pass.getResult();
    return m_output_fb;
}

void ForwardRenderer::endFrame() {
    m_skybox.reset();
    m_drawables.clear();
    m_lights.clear();
#ifndef NDEBUG
    // Only drain GL errors in debug builds; skip the per-frame glGetError round-trip in release.
    m_api->checkAndLogErrors();
#endif
    m_render_commands.clear();
}

void ForwardRenderer::resize(uint32_t width, uint32_t height) {
    m_api->setViewport(0, 0, width, height);
    m_geometry_pass.resize(width, height);
    // Resource descriptors are sized from the viewport, so the compiled graph is now stale. This is
    // the one path that must reliably re-dirty it -- RenderSystem::setViewport calls us on every
    // framebuffer resize.
    m_graph_dirty = true;
}

void ForwardRenderer::setClearColor(Eigen::Vector4f clearColor) {
    m_api->setClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
}

void ForwardRenderer::setViewport(int x, int y, int w, int h) {
    m_api->setViewport(x, y, w, h);
}

}  // namespace ICE
