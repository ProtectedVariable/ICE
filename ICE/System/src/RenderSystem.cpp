//
// Created by Thomas Ibanez on 19.11.20.
//

#include "RenderSystem.h"

namespace ICE {
void RenderSystem::update(double delta) {
    for (auto e : entities) {
        m_renderer->submit(e);
    }
    m_renderer->prepareFrame(*m_camera);
    m_renderer->render();
    m_renderer->endFrame();
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
    m_renderer->setTarget(fb);
}
}  // namespace ICE