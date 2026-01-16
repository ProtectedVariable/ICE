//
// Created by Thomas Ibanez on 19.11.20.
//

#include "RenderSystem.h"

#include "Registry.h"
#include "RenderData.h"

namespace ICE {
RenderSystem::RenderSystem(const std::shared_ptr<RendererAPI> &api, const std::shared_ptr<GraphicsFactory> &factory,
                           const std::shared_ptr<Registry> &reg, const std::shared_ptr<GPURegistry> &bank)
    : m_api(api),
      m_factory(factory),
      m_registry(reg),
      m_asset_bank(bank) {
    m_quad_vao = factory->createVertexArray();
    auto quad_vertex_vbo = factory->createVertexBuffer();
    quad_vertex_vbo->putData(full_quad_v.data(), full_quad_v.size() * sizeof(float));
    m_quad_vao->pushVertexBuffer(quad_vertex_vbo, 3);
    auto quad_uv_vbo = factory->createVertexBuffer();
    quad_uv_vbo->putData(full_quad_tx.data(), full_quad_tx.size() * sizeof(float));
    m_quad_vao->pushVertexBuffer(quad_uv_vbo, 2);
    auto quad_ibo = factory->createIndexBuffer();
    quad_ibo->putData(full_quad_idx.data(), full_quad_idx.size() * sizeof(int));
    m_quad_vao->setIndexBuffer(quad_ibo);
}

void RenderSystem::update(double delta) {

    auto view_mat = m_camera->lookThrough();
    auto proj_mat = m_camera->getProjection();

    if (m_skybox != NO_ASSET_ID) {
        auto shader = m_asset_bank->getShader(AssetPath::WithTypePrefix<Shader>("__ice_skybox_shader"));
        auto skybox = m_registry->getComponent<SkyboxComponent>(m_skybox);
        auto mesh = m_asset_bank->getMesh(AssetPath::WithTypePrefix<Mesh>("cube"));
        auto tex = m_asset_bank->getCubemap(skybox->texture);
        m_renderer->submitSkybox(Skybox{
            .cube_mesh = mesh,
            .shader = shader,
            .textures = {{skybox->texture, tex}},
        });
    }

    auto frustum = extractFrustumPlanes(proj_mat * view_mat);
    for (const auto &e : m_render_queue) {
        auto rc = m_registry->getComponent<RenderComponent>(e);
        auto tc = m_registry->getComponent<TransformComponent>(e);
        auto mesh = m_asset_bank->getMesh(rc->mesh);
        auto material = m_asset_bank->getMaterial(rc->material);
        auto shader = m_asset_bank->getShader(material->getShader());
        if (!mesh || !material || !shader)
            continue;

        auto model_mat = tc->getWorldMatrix();

        auto aabb = m_asset_bank->getMeshAABB(rc->mesh);
        Eigen::Vector3f min = (model_mat * Eigen::Vector4f(aabb.getMin().x(), aabb.getMin().y(), aabb.getMin().z(), 1.0)).head<3>();
        Eigen::Vector3f max = (model_mat * Eigen::Vector4f(aabb.getMax().x(), aabb.getMax().y(), aabb.getMax().z(), 1.0)).head<3>();
        aabb = AABB(std::vector<Eigen::Vector3f>{min, max});
        if (!isAABBInFrustum(frustum, aabb)) {
            continue;
        }

        std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> texs;
        for (const auto &[name, value] : material->getAllUniforms()) {
            if (std::holds_alternative<AssetUID>(value)) {
                auto v = std::get<AssetUID>(value);
                if (auto tex = m_asset_bank->getTexture2D(v); tex) {
                    texs.try_emplace(v, tex);
                }
            }
        }
        m_renderer->submitDrawable(Drawable{.mesh = mesh, .material = material, .shader = shader, .textures = texs, .model_matrix = model_mat});
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

    m_renderer->prepareFrame(*m_camera);
    auto rendered_fb = m_renderer->render();
    m_renderer->endFrame();

    //Final pass, render the last result to the screen
    if (!m_target) {
        m_api->bindDefaultFramebuffer();
    } else {
        m_target->bind();
    }

    m_api->clear();
    auto shader = m_asset_bank->getShader(AssetPath::WithTypePrefix<Shader>("lastpass"));

    shader->bind();
    rendered_fb->bindAttachment(0);
    shader->loadInt("uTexture", 0);
    m_quad_vao->bind();
    m_quad_vao->getIndexBuffer()->bind();
    m_api->renderVertexArray(m_quad_vao);
}

void RenderSystem::onEntityAdded(Entity e) {
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