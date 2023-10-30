#include "Assets.h"

Assets::Assets(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
}

bool Assets::update() {
    ui.render();
    return m_done;
}
