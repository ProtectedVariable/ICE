#include "Hierarchy.h"

#include <iostream>

Hierarchy::Hierarchy(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {
    ui.registerCallback("hierarchy_changed", [this](ICE::Entity child, ICE::Entity parent) {
        auto scene = m_engine->getProject()->getCurrentScene();
        scene->getGraph()->setParent(child, parent, true);
        m_need_rebuild_tree = true;
    });
    ui.registerCallback("create_entity_clicked", [this]() {
        auto scene = m_engine->getProject()->getCurrentScene();
        auto entity = scene->createEntity();
        m_need_rebuild_tree = true;
        //scene->getGraph()->setParent(e, ;
    });
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
