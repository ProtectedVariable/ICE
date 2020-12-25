//
// Created by Thomas Ibanez on 20.11.20.
//

#include <Scene/TransformComponent.h>
#include <Scene/LightComponent.h>
#include <iostream>
#include <GL/gl3w.h>
#include <Util/Logger.h>
#include "ForwardRenderer.h"

namespace ICE {

    void ForwardRenderer::initialize(RendererConfig config) {
        this->api = RendererAPI::Create();
        this->config = config;
    }

    void ForwardRenderer::submitScene(Scene *scene) {
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
        for(auto e : renderableEntities) {
            const Material* material = e->getComponent<RenderComponent>()->getMaterial();
            material->getShader()->bind();
            material->getShader()->loadMat4("projection", camera.getProjection());
            material->getShader()->loadMat4("view", camera.lookThrough());
            material->getShader()->loadFloat3("ambient_light", Eigen::Vector3f(0.3f,0.3f,0.3f));
            int i = 0;
            for(auto light : lightEntities) {
                std::string light_name = (std::string("lights[")+std::to_string(i)+std::string("]."));
                auto lc = light->getComponent<LightComponent>();
                material->getShader()->loadFloat3((light_name+std::string("position")).c_str(), *light->getComponent<TransformComponent>()->getPosition());
                material->getShader()->loadFloat3((light_name+std::string("rotation")).c_str(), *light->getComponent<TransformComponent>()->getRotation());
                material->getShader()->loadFloat3((light_name+std::string("color")).c_str(), lc->getColor());
                i++;
            }
            material->getShader()->loadInt("light_count", i);
        }
    }

    void ForwardRenderer::render() {
        for(auto e : renderableEntities) {
            const Material* mat = e->getComponent<RenderComponent>()->getMaterial();
            mat->getShader()->bind();
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
            mat->getShader()->loadMat4("model", e->getComponent<TransformComponent>()->getTransformation());
            mat->getShader()->loadFloat3("material.albedo", mat->getAlbedo());
            mat->getShader()->loadFloat3("material.specular", mat->getSpecular());
            mat->getShader()->loadFloat3("material.ambient", mat->getAmbient());
            mat->getShader()->loadFloat("material.alpha", mat->getAlpha());
            mat->getShader()->loadInt("material.use_diffuse_map", mat->getDiffuseMap() != nullptr);
            mat->getShader()->loadInt("material.diffuse_map", 0);
            mat->getShader()->loadInt("material.use_specular_map", mat->getSpecularMap() != nullptr);
            mat->getShader()->loadInt("material.specular_map", 1);
            mat->getShader()->loadInt("material.use_ambient_map", mat->getAmbientMap() != nullptr);
            mat->getShader()->loadInt("material.ambient_map", 2);
            mat->getShader()->loadInt("material.use_normal_map", mat->getNormalMap() != nullptr);
            mat->getShader()->loadInt("material.normal_map", 3);
            api->renderVertexArray(e->getComponent<RenderComponent>()->getMesh()->getVertexArray());
        }
    }

    void ForwardRenderer::endFrame() {
        unsigned int err;
        while((err = glGetError()) != GL_NO_ERROR){
            Logger::Log(Logger::ERROR, "Graphics", "OpenGL Error %d", err);
        }
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
}
