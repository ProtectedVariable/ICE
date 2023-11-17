#include "Inspector.h"

Inspector::Inspector(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
}

bool Inspector::update() {
    ui.render();
    return m_done;
}

void Inspector::setSelectedEntity(ICE::Entity e) {
    m_selected_entity = e;
    auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
    if (registry->entityHasComponent<ICE::TransformComponent>(e)) {
        auto tc = registry->getComponent<ICE::TransformComponent>(e);
    }
}
