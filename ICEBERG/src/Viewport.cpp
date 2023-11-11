#include "Viewport.h"

#include <iostream>

Viewport::Viewport(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    engine->setRenderFramebufferInternal(true);

    ui.registerCallback("w_pressed", [this]() { m_engine->getCamera()->forward(camera_delta); });
    ui.registerCallback("s_pressed", [this]() { m_engine->getCamera()->backward(camera_delta); });
    ui.registerCallback("a_pressed", [this]() { m_engine->getCamera()->left(camera_delta); });
    ui.registerCallback("d_pressed", [this]() { m_engine->getCamera()->right(camera_delta); });
    ui.registerCallback("ls_pressed", [this]() { m_engine->getCamera()->up(camera_delta); });
    ui.registerCallback("lc_pressed", [this]() { m_engine->getCamera()->down(camera_delta); });
    ui.registerCallback("mouse_dragged", [this](float dx, float dy) {
        m_engine->getCamera()->yaw(dx / 6.0);
        m_engine->getCamera()->pitch(dy / 6.0);
    });
    ui.registerCallback("resize", [this](float width, float height) {
        m_engine->getInternalFramebuffer()->resize(width, height);
        m_engine->getCamera()->resize(width, height);
    });
}

bool Viewport::update() {
    ui.setTexture(m_engine->getInternalFramebuffer()->getTexture());
    ui.render();
    return m_done;
}
