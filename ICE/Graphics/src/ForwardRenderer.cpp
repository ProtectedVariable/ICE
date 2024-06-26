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

namespace ICE {

ForwardRenderer::ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<Registry>& registry,
                                 const std::shared_ptr<AssetBank>& assetBank)
    : m_api(api),
      m_registry(registry),
      m_asset_bank(assetBank) {
}

void ForwardRenderer::submit(Entity e) {
    if (m_registry->entityHasComponent<RenderComponent>(e)) {
        auto rc = m_registry->getComponent<RenderComponent>(e);
        auto tc = m_registry->getComponent<TransformComponent>(e);
        m_render_queue.emplace_back(*rc, *tc);
    }
    if (m_registry->entityHasComponent<LightComponent>(e)) {
        m_lights.emplace_back(*m_registry->getComponent<LightComponent>(e), *m_registry->getComponent<TransformComponent>(e));
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
    m_api->clear();
    /* if (skybox->getTexture() != 0) {
        Skybox::getShader()->bind();
        Skybox::getShader()->loadMat4("projection", camera.getProjection());
        Eigen::Matrix4f view = camera.lookThrough();
        view(0, 3) = 0;
        view(1, 3) = 0;
        view(2, 3) = 0;
        Skybox::getShader()->loadMat4("view", view);
        Skybox::getShader()->loadInt("skybox", 0);
    }*/

    auto view_mat = camera.lookThrough();
    for (const auto& [rc, tc] : m_render_queue) {
        auto material = m_asset_bank->getAsset<Material>(rc.material);
        if (!material) {
            continue;
        }
        auto shader = m_asset_bank->getAsset<Shader>(material->getShader());
        if (!shader) {
            continue;
        }
        shader->bind();

        shader->loadMat4("projection", camera.getProjection());
        shader->loadMat4("view", view_mat);

        shader->loadFloat3("ambient_light", Eigen::Vector3f(0.1f, 0.1f, 0.1f));
        int i = 0;
        for (const auto& [light, position] : m_lights) {
            std::string light_name = (std::string("lights[") + std::to_string(i) + std::string("]."));
            shader->loadFloat3((light_name + std::string("position")).c_str(), position.position);
            shader->loadFloat3((light_name + std::string("rotation")).c_str(), position.rotation);
            shader->loadFloat3((light_name + std::string("color")).c_str(), light.color);
            i++;
        }
        shader->loadInt("light_count", i);

        m_render_commands.push_back([this, rc = rc, tc = tc] {
            auto material = m_asset_bank->getAsset<Material>(rc.material);
            auto shader = m_asset_bank->getAsset<Shader>(material->getShader());
            auto mesh = m_asset_bank->getAsset<Mesh>(rc.mesh);
            if (!mesh) {
                return;
            }
            shader->bind();

            shader->loadMat4("model", transformationMatrix(tc.position, tc.rotation, tc.scale));

            int texture_count = 0;
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
            m_api->renderVertexArray(mesh->getVertexArray());
        });
    }
}

void ForwardRenderer::render() {
    /* if (skybox->getTexture() != NO_ASSET_ID) {
        api->setDepthMask(false);
        Skybox::getShader()->bind();
        skybox->getVAO()->bind();
        assetBank->getAsset<TextureCube>(skybox->getTexture())->bind(0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }*/
    m_api->setDepthMask(true);
    for (const auto& cmd : m_render_commands) {
        cmd();
    }
}

void ForwardRenderer::endFrame() {
    m_api->checkAndLogErrors();
    //TODO: Cleanup and restore state
    m_render_commands.clear();
    m_render_queue.clear();
    m_lights.clear();
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
