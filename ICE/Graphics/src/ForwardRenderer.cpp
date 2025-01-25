//
// Created by Thomas Ibanez on 20.11.20.
//

#include "ForwardRenderer.h"

#include <GL/gl3w.h>
#include <ICEMath.h>
#include <LightComponent.h>
#include <Logger.h>
#include <RenderComponent.h>
#include <Scene.h>
#include <TransformComponent.h>

#include <unordered_set>

#include "RenderData.h"

namespace ICE {

ForwardRenderer::ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory,
                                 const std::shared_ptr<Registry>& registry, const std::shared_ptr<AssetBank>& assetBank)
    : m_api(api),
      m_registry(registry),
      m_asset_bank(assetBank),
      m_geometry_pass(api, factory, {1, 1, 1}) {

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

void ForwardRenderer::submit(Entity e) {
    remove(e);
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

void ForwardRenderer::remove(Entity e) {
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

void ForwardRenderer::prepareFrame(Camera& camera) {
    //TODO: Sort entities, make shader list, batch, make instances, set uniforms, etc..

    if (m_skybox != NO_ASSET_ID) {
        auto shader = m_asset_bank->getAsset<Shader>("__ice_skybox_shader");
        shader->bind();
        shader->loadMat4("projection", camera.getProjection());
        Eigen::Matrix4f view = camera.lookThrough();
        view(0, 3) = 0;
        view(1, 3) = 0;
        view(2, 3) = 0;
        shader->loadMat4("view", view);
        shader->loadInt("skybox", 0);

        auto skybox = m_registry->getComponent<SkyboxComponent>(m_skybox);
        auto mesh = m_asset_bank->getAsset<Mesh>("cube");
        auto tex = m_asset_bank->getAsset<TextureCube>(skybox->texture);
        m_render_commands.push_back(RenderCommand{.mesh = mesh,
                                                  .material = nullptr,
                                                  .shader = shader,
                                                  .textures = {{skybox->texture, tex}},
                                                  .faceCulling = false,
                                                  .depthTest = false});
    }

    std::unordered_set<AssetUID> prepared_shaders;
    auto view_mat = camera.lookThrough();
    auto frustum = extractFrustumPlanes(camera.getProjection() * view_mat);
    for (const auto& e : m_render_queue) {
        auto rc = m_registry->getComponent<RenderComponent>(e);
        auto tc = m_registry->getComponent<TransformComponent>(e);
        auto model = m_asset_bank->getAsset<Model>(rc->model);
        if (!model)
            continue;

        auto aabb = model->getBoundingBox();
        Eigen::Vector3f min = (tc->getModelMatrix() * Eigen::Vector4f(aabb.getMin().x(), aabb.getMin().y(), aabb.getMin().z(), 1.0)).head<3>();
        Eigen::Vector3f max = (tc->getModelMatrix() * Eigen::Vector4f(aabb.getMax().x(), aabb.getMax().y(), aabb.getMax().z(), 1.0)).head<3>();
        aabb = AABB(std::vector<Eigen::Vector3f>{min, max});
        if (!isAABBInFrustum(frustum, aabb)) {
            continue;
        }

        for (int i = 0; i < model->getMeshes().size(); i++) {
            auto mtl_id = model->getMaterialsIDs().at(i);
            auto mesh = model->getMeshes().at(i);
            auto material = m_asset_bank->getAsset<Material>(mtl_id);
            if (!material) {
                continue;
            }
            auto shader = m_asset_bank->getAsset<Shader>(material->getShader());
            if (!shader) {
                continue;
            }

            if (!prepared_shaders.contains(material->getShader())) {
                shader->bind();

                shader->loadMat4("projection", camera.getProjection());
                shader->loadMat4("view", view_mat);

                shader->loadFloat3("ambient_light", Eigen::Vector3f(0.1f, 0.1f, 0.1f));
                int i = 0;
                for (const auto& e : m_lights) {
                    auto light = m_registry->getComponent<LightComponent>(e);
                    auto transform = m_registry->getComponent<TransformComponent>(e);
                    std::string light_name = (std::string("lights[") + std::to_string(i) + std::string("]."));
                    shader->loadFloat3((light_name + std::string("position")).c_str(), transform->getPosition());
                    shader->loadFloat3((light_name + std::string("rotation")).c_str(), transform->getRotation());
                    shader->loadFloat3((light_name + std::string("color")).c_str(), light->color);
                    shader->loadInt((light_name + std::string("type")).c_str(), static_cast<int>(light->type));
                    i++;
                }
                shader->loadInt("light_count", i);
                prepared_shaders.emplace(material->getShader());
            }

            if (!mesh) {
                return;
            }

            std::unordered_map<AssetUID, std::shared_ptr<Texture>> texs;
            for (const auto& [name, value] : material->getAllUniforms()) {
                if (std::holds_alternative<AssetUID>(value)) {
                    auto v = std::get<AssetUID>(value);
                    if (auto tex = m_asset_bank->getAsset<Texture2D>(v); tex) {
                        texs.try_emplace(v, tex);
                    }
                }
            }

            m_render_commands.push_back(RenderCommand{.mesh = mesh,
                                                      .material = material,
                                                      .shader = shader,
                                                      .textures = texs,
                                                      .model_matrix = tc->getModelMatrix(),
                                                      .faceCulling = true,
                                                      .depthTest = true});
        }
    }

    std::sort(m_render_commands.begin(), m_render_commands.end(), [this](const RenderCommand &a, const RenderCommand &b) {
        bool a_transparent = a.material->isTransparent();
        bool b_transparent = b.material->isTransparent();

        if (!a_transparent && b_transparent) {
            return true;
        } else {
            return false;
        }
        });
    m_geometry_pass.submit(&m_render_commands);
}

void ForwardRenderer::render() {
    m_geometry_pass.execute();
    auto result = m_geometry_pass.getResult();

    //Final pass, render the last result to the screen
    if (!this->target) {
        m_api->bindDefaultFramebuffer();
    } else {
        this->target->bind();
    }
    m_api->clear();
    auto shader = m_asset_bank->getAsset<Shader>(AssetPath::WithTypePrefix<Shader>("lastpass"));

    shader->bind();
    result->bindAttachment(0);
    shader->loadInt("uTexture", 0);
    m_quad_vao->bind();
    m_quad_vao->getIndexBuffer()->bind();
    m_api->renderVertexArray(m_quad_vao);
}

void ForwardRenderer::endFrame() {
    m_api->checkAndLogErrors();
    //TODO: Cleanup and restore state
    m_render_commands.clear();

    if (this->target != nullptr)
        this->target->unbind();
}

void ForwardRenderer::setTarget(const std::shared_ptr<Framebuffer>& fb) {
    this->target = fb;
}

void ForwardRenderer::resize(uint32_t width, uint32_t height) {
    if (target) {
        target->bind();
        target->resize(width, height);
    }
    m_api->setViewport(0, 0, width, height);
    m_geometry_pass.resize(width, height);
}

void ForwardRenderer::setClearColor(Eigen::Vector4f clearColor) {
    m_api->setClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
}

void ForwardRenderer::setViewport(int x, int y, int w, int h) {
    m_api->setViewport(x, y, w, h);
}

}  // namespace ICE
