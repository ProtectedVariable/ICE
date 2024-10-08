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

namespace ICE {

ForwardRenderer::ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<Registry>& registry,
                                 const std::shared_ptr<AssetBank>& assetBank)
    : m_api(api),
      m_registry(registry),
      m_asset_bank(assetBank) {
}

void ForwardRenderer::submit(Entity e) {
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
    if (this->target != nullptr) {
        this->target->bind();
        m_api->setViewport(0, 0, target->getFormat().width, target->getFormat().height);
    } else {
        m_api->bindDefaultFramebuffer();
    }
    //TODO: Sort entities, make shader list, batch, make instances, set uniforms, etc..

    std::sort(m_render_queue.begin(), m_render_queue.end(), [this](Entity a, Entity b) {
        auto rc_a = m_registry->getComponent<RenderComponent>(a);
        auto material_a = m_asset_bank->getAsset<Material>(rc_a->material);
        auto rc_b = m_registry->getComponent<RenderComponent>(b);
        auto material_b = m_asset_bank->getAsset<Material>(rc_b->material);

        if (!material_a->isTransparent() && material_b->isTransparent()) {
            return true;
        } else {
            return false;
        }
    });

    m_api->clear();

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
    }

    std::unordered_set<AssetUID> prepared_shaders;
    auto view_mat = camera.lookThrough();
    for (const auto& e : m_render_queue) {
        auto rc = m_registry->getComponent<RenderComponent>(e);
        auto tc = m_registry->getComponent<TransformComponent>(e);
        auto material = m_asset_bank->getAsset<Material>(rc->material);
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
        m_render_commands.push_back([this, rc = rc, tc = tc] {
            auto material = m_asset_bank->getAsset<Material>(rc->material);
            auto shader = m_asset_bank->getAsset<Shader>(material->getShader());
            auto mesh = m_asset_bank->getAsset<Mesh>(rc->mesh);
            if (!mesh) {
                return;
            }

            if (material->getShader() != m_current_shader) {
                shader->bind();
                m_current_shader = material->getShader();
            }

            int texture_count = 0;

            shader->loadMat4("model", tc->getModelMatrix());

            if (rc->material != m_current_material) {

                m_current_material = rc->material;

                //TODO: Can we do better ?
                for (const auto& [name, value] : material->getAllUniforms()) {
                    if (std::holds_alternative<float>(value)) {
                        auto v = std::get<float>(value);
                        shader->loadFloat(name, v);
                    } else if (!std::holds_alternative<AssetUID>(value) && std::holds_alternative<int>(value)) {
                        auto v = std::get<int>(value);
                        shader->loadInt(name, v);
                    } else if (std::holds_alternative<AssetUID>(value)) {
                        auto v = std::get<AssetUID>(value);
                        if (auto tex = m_asset_bank->getAsset<Texture2D>(v); tex) {
                            tex->bind(texture_count);
                            shader->loadInt(name, texture_count);
                            texture_count++;
                        }
                    } else if (std::holds_alternative<Eigen::Vector2f>(value)) {
                        auto& v = std::get<Eigen::Vector2f>(value);
                        shader->loadFloat2(name, v);
                    } else if (std::holds_alternative<Eigen::Vector3f>(value)) {
                        auto& v = std::get<Eigen::Vector3f>(value);
                        shader->loadFloat3(name, v);
                    } else if (std::holds_alternative<Eigen::Vector4f>(value)) {
                        auto& v = std::get<Eigen::Vector4f>(value);
                        shader->loadFloat4(name, v);
                    } else if (std::holds_alternative<Eigen::Matrix4f>(value)) {
                        auto& v = std::get<Eigen::Matrix4f>(value);
                        shader->loadMat4(name, v);
                    } else {
                        throw std::runtime_error("Uniform type not implemented");
                    }
                }
            }

            if (m_current_mesh != rc->mesh) {
                m_current_mesh = rc->mesh;
                auto va = mesh->getVertexArray();
                va->bind();
                va->getIndexBuffer()->bind();
            }

            m_api->renderVertexArray(mesh->getVertexArray());
        });
    }
}

void ForwardRenderer::render() {
    if (m_skybox != NO_ASSET_ID) {
        m_api->setBackfaceCulling(false);
        m_api->setDepthMask(false);
        auto skybox = m_registry->getComponent<SkyboxComponent>(m_skybox);
        auto shader = m_asset_bank->getAsset<Shader>("__ice_skybox_shader");
        shader->bind();
        if (skybox->texture != NO_ASSET_ID) {
            m_asset_bank->getAsset<TextureCube>(skybox->texture)->bind(0);
        }
        auto vao = m_asset_bank->getAsset<Mesh>("cube")->getVertexArray();
        vao->bind();
        vao->getIndexBuffer()->bind();
        m_api->renderVertexArray(vao);
    }
    m_api->setBackfaceCulling(true);
    m_api->setDepthMask(true);
    for (const auto& cmd : m_render_commands) {
        cmd();
    }
}

void ForwardRenderer::endFrame() {
    m_api->checkAndLogErrors();
    //TODO: Cleanup and restore state
    m_render_commands.clear();
    m_current_material = 0;
    m_current_mesh = 0;
    m_current_shader = 0;
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
}

void ForwardRenderer::setClearColor(Eigen::Vector4f clearColor) {
    m_api->setClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
}
}  // namespace ICE
