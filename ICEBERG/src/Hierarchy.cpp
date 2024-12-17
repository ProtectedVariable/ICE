#include "Hierarchy.h"

#include <iostream>

Hierarchy::Hierarchy(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {
    ui.registerCallback("hierarchy_changed", [this](ICE::Entity child, ICE::Entity parent) {
        auto scene = m_engine->getProject()->getCurrentScene();
        scene->getGraph()->setParent(child, parent, true);
        m_need_rebuild_tree = true;
    });
    ui.registerCallback("create_entity_clicked", [this](ICE::Entity parent) {
        auto scene = m_engine->getProject()->getCurrentScene();
        auto entity = scene->createEntity();
        scene->getRegistry()->addComponent<ICE::TransformComponent>(
            entity, ICE::TransformComponent(Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero(), Eigen::Vector3f(1, 1, 1)));

        auto cube_id = m_engine->getAssetBank()->getUID(ICE::AssetPath::WithTypePrefix<ICE::Mesh>("cube"));
        auto mat_id = m_engine->getAssetBank()->getUID(ICE::AssetPath::WithTypePrefix<ICE::Material>("base_mat"));
        scene->getRegistry()->addComponent<ICE::RenderComponent>(entity, ICE::RenderComponent(cube_id, mat_id));

        scene->getGraph()->setParent(entity, parent, false);
        m_need_rebuild_tree = true;
    });
    ui.registerCallback("selected_entity_changed", [this](ICE::Entity selected) { m_selected = selected; });
}

SceneTreeView getSubTree(const std::shared_ptr<ICE::Scene> &scene, const std::shared_ptr<ICE::SceneGraph::SceneNode> &node) {
    SceneTreeView view;

    for (const auto &c : node->children) {
        view.children.push_back(std::make_shared<SceneTreeView>(getSubTree(scene, c)));
    }
    view.id = node->entity;
    view.entity_name = scene->getAlias(node->entity);
    return view;
}

SceneTreeView Hierarchy::getTreeView(const std::shared_ptr<ICE::Scene> &scene) const {
    auto graph = scene->getGraph();
    return getSubTree(scene, graph->root);
}

ICE::Entity Hierarchy::getSelectedEntity() const {
    return m_selected;
}

void Hierarchy::setSelectedEntity(ICE::Entity e) {
    m_selected = e;
}

void Hierarchy::rebuildTree() {
    m_need_rebuild_tree = true;
}

bool Hierarchy::update() {
    if (m_need_rebuild_tree) {
        auto scene = m_engine->getProject()->getCurrentScene();
        auto view = getTreeView(scene);
        ui.setSceneTree(view);
        m_need_rebuild_tree = false;
    }
    ui.render();
    return m_done;
}
