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

    std::vector<Signature> getSignatures(const ComponentManager &comp_manager) const override {
        Signature signature0;
        signature0.set(comp_manager.getComponentType<TransformComponent>());
        return {signature0};
    }

   private:
    std::shared_ptr<Scene> m_scene;
};
}  // namespace ICE
