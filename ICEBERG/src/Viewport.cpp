#include "Viewport.h"

Viewport::Viewport(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
}

bool Viewport::update() {
    ui.setTexture(m_engine->getInternalFramebuffer()->getTexture());
    ui.render();
    return m_done;
}
