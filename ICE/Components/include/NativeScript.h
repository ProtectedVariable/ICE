#pragma once

#include <Entity.h>

namespace ICE {
class Registry;

// Base class for native (C++) per-entity game logic. Subclass it, override the lifecycle
// hooks, and attach it to an entity through NativeScriptComponent::bind<T>(). The ScriptSystem
// instantiates one instance per entity and drives its lifecycle.
class NativeScript {
   public:
    virtual ~NativeScript() = default;

    // Called once, right after the instance is attached to its entity.
    virtual void onCreate() {}
    // Called every frame with the frame delta in seconds. Scripts run first each frame
    // (SystemUpdateOrder::ScriptSystemOrder), so transform changes made here are picked up by
    // animation, the scene graph and rendering the same frame.
    virtual void onUpdate(double dt) {}
    // Called once, right before the instance is detached (component or entity removed).
    virtual void onDestroy() {}

   protected:
    // The entity this script drives and its registry, for component access:
    //   registry()->getComponent<TransformComponent>(entity())->setPosition(...);
    Entity entity() const { return m_entity; }
    Registry* registry() const { return m_registry; }

   private:
    friend class ScriptSystem;  // injects m_entity/m_registry before onCreate()
    Entity m_entity = NULL_ENTITY;
    Registry* m_registry = nullptr;
};
}  // namespace ICE
