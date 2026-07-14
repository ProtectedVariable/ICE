#pragma once

#include <NativeScriptComponent.h>
#include <Registry.h>

#include <memory>
#include <unordered_map>

#include "System.h"

namespace ICE {

// Drives NativeScriptComponent lifecycles: instantiates one NativeScript per entity and calls
// onCreate/onUpdate/onDestroy. Runs first each frame (ScriptSystemOrder) so gameplay changes to
// transforms are visible to animation, the scene graph and rendering the same frame. The live
// instances are owned here (keyed by entity), not in the component.
class ScriptSystem : public System {
   public:
    explicit ScriptSystem(const std::shared_ptr<Registry>& reg) : m_registry(reg.get()) {}

    void update(double delta) override;
    void onEntityAdded(Entity e) override;
    void onEntityRemoved(Entity e) override;

    int updateOrder() const override { return ScriptSystemOrder; }

    std::vector<Signature> getSignatures(const ComponentManager& comp_manager) const override {
        Signature signature;
        signature.set(comp_manager.getComponentType<NativeScriptComponent>());
        return {signature};
    }

   private:
    // Returns the entity's live script, instantiating (and onCreate-ing) it on first use.
    // nullptr if the entity has no bound script.
    NativeScript* ensureInstance(Entity e);

    // Non-owning back-reference (the Registry owns this system) to avoid an ownership cycle.
    Registry* m_registry = nullptr;
    std::unordered_map<Entity, std::shared_ptr<NativeScript>> m_instances;
};
}  // namespace ICE
