#pragma once

#include <Scene.h>

#include "System.h"

namespace ICE {

class SceneGraphSystem : public System {
   public:
    SceneGraphSystem(const std::shared_ptr<Scene> &scene);

    void onEntityAdded(Entity e) override;
    void onEntityRemoved(Entity e) override;
    void update(double delta) override;

    int updateOrder() const override { return SceneGraphSystemOrder; }

    // Recursive transform propagation over raw node pointers (no per-frame std::function
    // allocation, no shared_ptr refcount churn per node).
    void updateNode(SceneGraph::SceneNode *node, const Eigen::Matrix4f &parentMatrix, bool parent_changed);

    std::vector<Signature> getSignatures(const ComponentManager &comp_manager) const override {
        Signature signature0;
        signature0.set(comp_manager.getComponentType<TransformComponent>());
        return {signature0};
    }

   private:
    // Non-owning: the Scene owns this system's Registry (which owns this system), so holding
    // a shared_ptr here formed a Scene -> Registry -> SystemManager -> this -> Scene cycle
    // that leaked the whole scene. The Scene always outlives its systems.
    Scene* m_scene = nullptr;
    std::unordered_map<Entity, uint32_t> m_transformVersions;
};
}  // namespace ICE
