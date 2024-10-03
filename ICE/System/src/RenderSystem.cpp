//
// Created by Thomas Ibanez on 19.11.20.
//

#include "RenderSystem.h"

namespace ICE {
void RenderSystem::update(double delta) {
    m_renderer->prepareFrame(*m_camera);
    m_renderer->render();
    m_renderer->endFrame();
}

void RenderSystem::onEntityAdded(Entity e) {
    if (m_renderer) {
        m_renderer->submit(e);
    }
}

void RenderSystem::onEntityRemoved(Entity e) {
    if (m_renderer) {
        m_renderer->remove(e);
    }
}

std::shared_ptr<Renderer> RenderSystem::getRenderer() const {
    return m_renderer;
}

void RenderSystem::setRenderer(const std::shared_ptr<Renderer> &renderer) {
    m_renderer = renderer;
    for (const auto &e : entities) {
        m_renderer->submit(e);
    }
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

void RenderSystem::setViewport(int x, int y, int w, int h) {
    m_renderer->resize(w, h);
}

}  // namespace ICE