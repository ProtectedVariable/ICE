//
// Created by Thomas Ibanez on 20.11.20.
//

#include <GL/gl3w.h>
#include "ForwardRenderer.h"
#include <Scene/TransformComponent.h>
#include <Scene/LightComponent.h>
#include <Util/Logger.h>
#include <Scene/Scene.h>
#include <Scene/RenderComponent.h>

namespace ICE {

    void ForwardRenderer::initialize(RendererConfig config) {
        this->api = RendererAPI::Create();
        this->config = config;
    }

    void ForwardRenderer::submitScene(Scene *scene) {
        skybox = scene->getSkybox();
        for(auto e : scene->getEntities()) {
            submit(e);
        }
    }

    void ForwardRenderer::submit(Entity *e) {
        if(e->hasComponent<RenderComponent>() && e->hasComponent<TransformComponent>()) {
            renderableEntities.push_back(e);
        }
        if(e->hasComponent<LightComponent>() && e->hasComponent<TransformComponent>()) {
            lightEntities.push_back(e);
        }
    }

    void ForwardRenderer::prepareFrame(Camera& camera) {
        if(this->target != nullptr)
            this->target->bind();
        else
            api->bindDefaultFramebuffer();
        //TODO: Sort entities, make shader list, batch, make instances, set uniforms, etc..
        api->clear();
        if(skybox != nullptr) {
            Skybox::getShader()->bind();
            Skybox::getShader()->loadMat4("projection", camera.getProjection());
            Eigen::Matrix4f view = camera.lookThrough();
            view(0, 3) = 0;
            view(1, 3) = 0;
            view(2, 3) = 0;
            Skybox::getShader()->loadMat4("view", view);
            Skybox::getShader()->loadInt("skybox", 0);
        }
        for(auto e : renderableEntities) {
            const Material* material = e->getComponent<RenderComponent>()->getMaterial();
            Shader* shader = e->getComponent<RenderComponent>()->getShader();
            shader->bind();
            shader->loadMat4("projection", camera.getProjection());
            shader->loadMat4("view", camera.lookThrough());
            shader->loadFloat3("ambient_light", Eigen::Vector3f(0.1f,0.1f,0.1f));
            int i = 0;
            for(auto light : lightEntities) {
                std::string light_name = (std::string("lights[")+std::to_string(i)+std::string("]."));
                auto lc = light->getComponent<LightComponent>();
                shader->loadFloat3((light_name+std::string("position")).c_str(), *light->getComponent<TransformComponent>()->getPosition());
                shader->loadFloat3((light_name+std::string("rotation")).c_str(), *light->getComponent<TransformComponent>()->getRotation());
                shader->loadFloat3((light_name+std::string("color")).c_str(), lc->getColor());
                i++;
            }
            shader->loadInt("light_count", i);
        }
    }

    void ForwardRenderer::render() {
        if(skybox->getTexture() != nullptr) {
            api->setDepthMask(false);
            Skybox::getShader()->bind();
            skybox->getVAO()->bind();
            skybox->getTexture()->bind(0);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        api->setDepthMask(true);
        for(auto e : renderableEntities) {
            const Material* mat = e->getComponent<RenderComponent>()->getMaterial();
            Shader* shader = e->getComponent<RenderComponent>()->getShader();
            shader->bind();
            if(mat->getDiffuseMap() != nullptr) {
                mat->getDiffuseMap()->bind(0);
            }
            if(mat->getSpecularMap() != nullptr) {
                mat->getSpecularMap()->bind(1);
            }
            if(mat->getAmbientMap() != nullptr) {
                mat->getAmbientMap()->bind(2);
            }
            if(mat->getNormalMap() != nullptr) {
                mat->getNormalMap()->bind(3);
            }
            shader->loadMat4("model", e->getComponent<TransformComponent>()->getTransformation());
            shader->loadFloat3("material.albedo", mat->getAlbedo());
            shader->loadFloat3("material.specular", mat->getSpecular());
            shader->loadFloat3("material.ambient", mat->getAmbient());
            shader->loadFloat("material.alpha", mat->getAlpha());
            shader->loadInt("material.use_diffuse_map", mat->getDiffuseMap() != nullptr);
            shader->loadInt("material.diffuse_map", 0);
            shader->loadInt("material.use_specular_map", mat->getSpecularMap() != nullptr);
            shader->loadInt("material.specular_map", 1);
            shader->loadInt("material.use_ambient_map", mat->getAmbientMap() != nullptr);
            shader->loadInt("material.ambient_map", 2);
            shader->loadInt("material.use_normal_map", mat->getNormalMap() != nullptr);
            shader->loadInt("material.normal_map", 3);
            api->renderVertexArray(e->getComponent<RenderComponent>()->getMesh()->getVertexArray());
        }
    }

    void ForwardRenderer::endFrame() {
        api->checkAndLogErrors();
        renderableEntities.clear();
        lightEntities.clear();
        //TODO: Cleanup and restore state
        if(this->target != nullptr)
            this->target->unbind();
    }

    void ForwardRenderer::setTarget(Framebuffer* target) {
        this->target = target;
    }

    void ForwardRenderer::resize(uint32_t width, uint32_t height) {
        this->target->bind();
        this->target->resize(width, height);
        this->api->setViewport(0, 0, width, height);
    }

    void ForwardRenderer::setClearColor(Eigen::Vector4f clearColor) {
        this->api->setClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
    }
}
