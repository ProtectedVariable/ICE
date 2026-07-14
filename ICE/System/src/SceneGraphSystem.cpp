#include "SceneGraphSystem.h"

namespace ICE {
SceneGraphSystem::SceneGraphSystem(const std::shared_ptr<Scene> &scene) : m_scene(scene.get()) {
}

void SceneGraphSystem::onEntityAdded(Entity e) {
}
void SceneGraphSystem::onEntityRemoved(Entity e) {
    // Evict the cached transform version so a recycled entity id isn't skipped because its
    // new version happens to match the stale cached one.
    m_transformVersions.erase(e);
}
void SceneGraphSystem::update(double delta) {
    updateNode(m_scene->getGraph()->getRoot().get(), Eigen::Matrix4f::Identity(), false);
}

void SceneGraphSystem::updateNode(SceneGraph::SceneNode *node, const Eigen::Matrix4f &parentMatrix, bool parent_changed) {
    auto *registry = m_scene->getRegistry().get();
    Eigen::Matrix4f newParentMatrix = parentMatrix;
    if (node->entity != NULL_ENTITY && registry->entityHasComponent<TransformComponent>(node->entity)) {
        auto tc = registry->getComponent<TransformComponent>(node->entity);

        if (parent_changed) {
            tc->updateParentMatrix(parentMatrix);
        }

        auto it = m_transformVersions.find(node->entity);
        if (it == m_transformVersions.end() || it->second != tc->getVersion()) {
            parent_changed = true;
            m_transformVersions[node->entity] = tc->getVersion();
        }
        newParentMatrix = tc->getWorldMatrix();
    }
    for (const auto &child : node->children) {
        updateNode(child.get(), newParentMatrix, parent_changed);
    }
}

}  // namespace ICE