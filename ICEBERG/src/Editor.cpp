#include "Editor.h"

#include "Assets.h"
#include "Hierarchy.h"
#include "Inspector.h"
#include "Viewport.h"

Editor::Editor(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    m_subpannels.push_back(std::make_unique<Viewport>(engine));
    m_subpannels.push_back(std::make_unique<Hierarchy>(engine));
    m_subpannels.push_back(std::make_unique<Inspector>(engine));
    m_subpannels.push_back(std::make_unique<Assets>(engine));
}

bool Editor::update() {
    ui.render();
    for (const auto& pannel : m_subpannels) {
        pannel->update();
    }
    return m_done;
}