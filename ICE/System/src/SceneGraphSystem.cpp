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
    std::function<void(const std::shared_ptr<SceneGraph::SceneNode> &, const Eigen::Matrix4f &)> updateNode;
    updateNode = [this, &updateNode](const std::shared_ptr<SceneGraph::SceneNode> &node, const Eigen::Matrix4f &parentMatrix) {
        Eigen::Matrix4f newParentMatrix = parentMatrix;
        if (node->entity != 0 && m_scene->getRegistry()->entityHasComponent<TransformComponent>(node->entity)) {
            auto tc = m_scene->getRegistry()->getComponent<TransformComponent>(node->entity);
            tc->updateParentMatrix(parentMatrix);
            newParentMatrix = tc->getWorldMatrix();
        }
        for (const auto &child : node->children) {
            updateNode(child, newParentMatrix);
        }
    };
    updateNode(root, Eigen::Matrix4f::Identity());
}

}  // namespace ICE