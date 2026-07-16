#pragma once

#include <NativeScriptComponent.h>
#include <Registry.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "System.h"

namespace ICE {
class Scene;          // injected into scripts as scene(); only stored/threaded here (never derefed)
class InputManager;   // injected into scripts as input(); only stored/threaded here (never derefed)

// Drives NativeScriptComponent lifecycles: instantiates one NativeScript per entity, injects its
// behaviour context (entity/registry/scene/input/time) and calls onCreate/onUpdate/onDestroy. Runs
// first each frame (ScriptSystemOrder) so gameplay changes to transforms are visible to animation,
// the scene graph and rendering the same frame. The live instances are owned here (keyed by
// entity), not in the component.
class ScriptSystem : public System {
   public:
    // scene/input become every script's behaviour context; both may be null (a scene with no
    // engine input service, or a headless test), in which case scene()/input() return null.
    ScriptSystem(const std::shared_ptr<Registry>& reg, Scene* scene = nullptr, InputManager* input = nullptr)
        : m_registry(reg.get()), m_scene(scene), m_input(input) {}

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
    Scene* m_scene = nullptr;         // non-owning; injected into scripts as scene()
    InputManager* m_input = nullptr;  // non-owning; injected into scripts as input()
    double m_elapsed = 0.0;           // accumulated frame time, injected into scripts as time()
    std::unordered_map<Entity, std::shared_ptr<NativeScript>> m_instances;
    // Entities whose script called destroy() this frame: removed after the onUpdate pass so a
    // script can't be torn down under its own feet mid-update.
    std::vector<Entity> m_pending_destroy;
};
}  // namespace ICE
