#include "Inspector.h"

Inspector::Inspector(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
}

bool Inspector::update() {
    ui.render();
    return m_done;
}
