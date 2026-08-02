#include "Hierarchy.h"

#include <AnimationComponent.h>
#include <LightComponent.h>
#include <RenderComponent.h>
#include <SkeletonPoseComponent.h>
#include <SkinningComponent.h>
#include <SkyboxComponent.h>
#include <TransformComponent.h>

#include <iostream>

namespace {
// Copy component T from src to dst if src has it. addComponent takes T by value, so *src is
// copied into the parameter before insertData runs -- safe against storage reallocation.
template<typename T>
void copyComponent(const std::shared_ptr<ICE::Registry> &reg, ICE::Entity src, ICE::Entity dst) {
    if (reg->entityHasComponent<T>(src)) {
        reg->addComponent<T>(dst, *reg->getComponent<T>(src));
    }
}
}  // namespace

Hierarchy::Hierarchy(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {
    ui.registerCallback("hierarchy_changed", [this](ICE::Entity child, ICE::Entity parent) {
        auto scene = m_engine->getProject()->getCurrentScene();
        scene->getGraph()->setParent(child, parent);
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

        scene->getGraph()->setParent(entity, parent);
        m_need_rebuild_tree = true;
    });
    ui.registerCallback("selected_entity_changed", [this](ICE::Entity selected) { m_selected = selected; });
    ui.registerCallback("delete_entity_clicked", [this](ICE::Entity e) {
        auto scene = m_engine->getProject()->getCurrentScene();
        if (e == ICE::NULL_ENTITY || !scene->hasEntity(e)) {
            return;
        }
        scene->removeEntity(e);
        // The whole subtree goes with e, so check aliveness rather than identity: the selection
        // may have been one of the deleted descendants.
        if (m_selected != ICE::NULL_ENTITY && !scene->hasEntity(m_selected)) {
            setSelectedEntity(ICE::NULL_ENTITY);
        }
        m_need_rebuild_tree = true;
    });
    ui.registerCallback("duplicate_entity_clicked", [this](ICE::Entity src) {
        auto scene = m_engine->getProject()->getCurrentScene();
        if (src == ICE::NULL_ENTITY || !scene->hasEntity(src)) {
            return;
        }
        auto reg = scene->getRegistry();
        auto e = scene->createEntity();
        // Copy every component the source carries. addComponent takes the component by value,
        // so the source pointer is dereferenced and copied before any storage reallocation.
        copyComponent<ICE::TransformComponent>(reg, src, e);
        copyComponent<ICE::RenderComponent>(reg, src, e);
        copyComponent<ICE::LightComponent>(reg, src, e);
        copyComponent<ICE::SkyboxComponent>(reg, src, e);
        copyComponent<ICE::AnimationComponent>(reg, src, e);
        copyComponent<ICE::SkeletonPoseComponent>(reg, src, e);
        copyComponent<ICE::SkinningComponent>(reg, src, e);
        scene->setAlias(e, scene->getAlias(src) + " copy");
        scene->getGraph()->setParent(e, scene->getGraph()->getParentID(src));
        setSelectedEntity(e);
        m_need_rebuild_tree = true;
    });
    ui.registerCallback("rename_entity", [this](ICE::Entity e, std::string name) {
        auto scene = m_engine->getProject()->getCurrentScene();
        if (e == ICE::NULL_ENTITY || !scene->hasEntity(e) || name.empty()) {
            return;
        }
        scene->setAlias(e, name);
        m_need_rebuild_tree = true;
        if (e == m_selected) {
            m_selection_renamed = true;
        }
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
    return getSubTree(scene, graph->getRoot());
}

ICE::Entity Hierarchy::getSelectedEntity() const {
    return m_selected;
}

void Hierarchy::setSelectedEntity(ICE::Entity e) {
    m_selected = e;
    ui.selected_id = e;
}

void Hierarchy::rebuildTree() {
    m_need_rebuild_tree = true;
}

bool Hierarchy::selectionRenamed() {
    bool renamed = m_selection_renamed;
    m_selection_renamed = false;
    return renamed;
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
