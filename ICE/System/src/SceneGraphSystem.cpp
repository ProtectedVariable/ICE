#include "SceneGraphSystem.h"

namespace ICE {
SceneGraphSystem::SceneGraphSystem(const std::shared_ptr<Scene> &scene) : m_scene(scene) {

}

void SceneGraphSystem::onEntityAdded(Entity e) {
}
void SceneGraphSystem::onEntityRemoved(Entity e) {
}
void SceneGraphSystem::update(double delta) {
    auto root = m_scene->getGraph()->getRoot();
    std::function<void(const std::shared_ptr<SceneGraph::SceneNode> &, Eigen::Matrix4f &)> updateNode;
    updateNode = [this, &updateNode](const std::shared_ptr<SceneGraph::SceneNode> &node, Eigen::Matrix4f &parentMatrix) {
        if (node->entity != 0 && m_scene->getRegistry()->entityHasComponent<TransformComponent>(node->entity)) {
            auto tc = m_scene->getRegistry()->getComponent<TransformComponent>(node->entity);
            tc->updateParentMatrix(parentMatrix);
            parentMatrix = tc->getWorldMatrix();
        }
        for (const auto &child : node->children) {
            updateNode(child, parentMatrix);
        }
    };
    Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();
    updateNode(root, identity);
}

}  // namespace ICE