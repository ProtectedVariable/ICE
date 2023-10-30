#include "Hierarchy.h"

Hierarchy::Hierarchy(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {}

bool Hierarchy::update() {
    ui.render();
    return m_done;
}
