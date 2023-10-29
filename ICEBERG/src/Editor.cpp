#include "Editor.h"

Editor::Editor(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {
}

bool Editor::update() {
    ui.render();
    return m_done;
}