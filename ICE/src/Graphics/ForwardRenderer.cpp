//
// Created by Thomas Ibanez on 20.11.20.
//

#include <Scene/TransformComponent.h>
#include <Scene/LightComponent.h>
#include <iostream>
#include <GL/gl3w.h>
#include "ForwardRenderer.h"

namespace ICE {

    void ForwardRenderer::initialize(RendererAPI *api, RendererConfig config) {
        this->api = api;
        this->config = config;
        api->initialize();
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

    void ForwardRenderer::prepareFrame() {
        //TODO: Sort entities, make shader list, batch, make instances, set uniforms, etc..
    }

    void ForwardRenderer::render() {
        for(auto e : renderableEntities) {
            e->getComponent<RenderComponent>()->getMaterial()->getShader()->bind();
            api->renderVertexArray(e->getComponent<RenderComponent>()->getMesh()->getVertexArray());
        }
    }

    void ForwardRenderer::endFrame() {
        unsigned int err = 0;
        while((err = glGetError()) != GL_NO_ERROR){
            std::cout << err << std::endl;
        }
        //TODO: Cleanup and restore state
    }

    void ForwardRenderer::setCamera(const Camera *camera) {
        this->currentCamera = camera;
    }

    void ForwardRenderer::setTarget(FrameBuffer *target) {

    }

    void ForwardRenderer::resize(uint32_t width, uint32_t height) {

    }
}
