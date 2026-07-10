#include "SceneGraphSystem.h"

namespace ICE {
SceneGraphSystem::SceneGraphSystem(const std::shared_ptr<Scene> &scene) : m_scene(scene) {
}

void SceneGraphSystem::onEntityAdded(Entity e) {
}
void SceneGraphSystem::onEntityRemoved(Entity e) {
    // Evict the cached transform version so a recycled entity id isn't skipped because its
    // new version happens to match the stale cached one.
    m_transformVersions.erase(e);
}
void SceneGraphSystem::update(double delta) {
    auto root = m_scene->getGraph()->getRoot();
    std::function<void(const std::shared_ptr<SceneGraph::SceneNode> &, const Eigen::Matrix4f &, bool)> updateNode;
    updateNode = [this, &updateNode](const std::shared_ptr<SceneGraph::SceneNode> &node, const Eigen::Matrix4f &parentMatrix, bool parent_changed) {
        Eigen::Matrix4f newParentMatrix = parentMatrix;
        if (node->entity != 0 && m_scene->getRegistry()->entityHasComponent<TransformComponent>(node->entity)) {
            auto tc = m_scene->getRegistry()->getComponent<TransformComponent>(node->entity);

            if (parent_changed) {
                tc->updateParentMatrix(parentMatrix);
            }

            if (!m_transformVersions.contains(node->entity) || m_transformVersions[node->entity] != tc->getVersion()) {
                parent_changed = true;
                m_transformVersions[node->entity] = tc->getVersion();
            }
            newParentMatrix = tc->getWorldMatrix();

        }
        for (const auto &child : node->children) {
            updateNode(child, newParentMatrix, parent_changed);
        }
    };
    updateNode(root, Eigen::Matrix4f::Identity(), false);
}

}  // namespace ICE