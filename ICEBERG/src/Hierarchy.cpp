#include "Hierarchy.h"

Hierarchy::Hierarchy(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    SceneTreeView subtree1 = {"Entity 1", EntityType::Renderable, {}};
    SceneTreeView subtree2 = {"Entity 2", EntityType::Camera, {std::make_shared<SceneTreeView>(subtree1)}};
    SceneTreeView subtree = {"Entity 3", EntityType::LightSource, {}};
    ui.setSceneTree({"Scene", EntityType::Scene, {std::make_shared<SceneTreeView>(subtree), std::make_shared<SceneTreeView>(subtree2)}});
}

bool Hierarchy::update() {
    ui.render();
    return m_done;
}
