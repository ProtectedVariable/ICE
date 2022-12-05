//
// Created by Thomas Ibanez on 19.11.20.
//

#include "RenderSystem.h"

namespace ICE {
    void RenderSystem::update(Scene* scene, double delta) {
        renderer->submitScene(scene);
        renderer->prepareFrame(*camera);
        renderer->render();
        renderer->endFrame();
    }

    void RenderSystem::entitySignatureChanged(Entity e) {

    }


    RenderSystem::RenderSystem(Renderer *renderer, Camera* camera) : renderer(renderer), camera(camera) {}

    Renderer *RenderSystem::getRenderer() const {
        return renderer;
    }

    void RenderSystem::setRenderer(Renderer *renderer) {
        RenderSystem::renderer = renderer;
    }

    Camera* RenderSystem::getCamera() const {
        return camera;
    }

    void RenderSystem::setCamera(Camera *camera) {
        RenderSystem::camera = camera;
    }

    void RenderSystem::setTarget(Framebuffer *fb, int width, int height) {
        renderer->setTarget(fb);
        renderer->resize(width, height);
    }
}