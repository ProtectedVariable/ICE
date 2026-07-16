#pragma once

#include <EntityHandle.h>

#include <utility>

namespace ICE {
// Forward-declared (used only as pointers here); a script that actually dereferences them includes
// the header itself: <Scene.h> for instantiate(), <InputManager.h> to read input().
class Scene;
class InputManager;

// Base class for native (C++) per-entity game logic. Subclass it, override the lifecycle hooks, and
// attach it to an entity through EntityHandle::script<T>() (or NativeScriptComponent::bind<T>()).
// The ScriptSystem instantiates one instance per entity, injects its behaviour context, and drives
// its lifecycle.
//
// Inside the hooks the entity reads as intent -- no registry plumbing:
//   self()                 // an EntityHandle to this entity
//   transform()            // this entity's TransformComponent (nullptr if none)
//   getComponent<T>()      // any component on this entity (nullptr if absent)
//   scene(), input()       // the scene this lives in / the engine input service
//   time()                 // seconds elapsed since scripting started
//   instantiate(...)       // spawn another entity into the scene
//   destroy()              // remove this entity (deferred to end of frame -- safe from onUpdate)
class NativeScript {
   public:
    virtual ~NativeScript() = default;

    // Called once, right after the instance is attached to its entity (context already injected).
    virtual void onCreate() {}
    // Called every frame with the frame delta in seconds. Scripts run first each frame
    // (SystemUpdateOrder::ScriptSystemOrder), so transform changes made here are picked up by
    // animation, the scene graph and rendering the same frame.
    virtual void onUpdate(double dt) {}
    // Called once, right before the instance is detached (component or entity removed).
    virtual void onDestroy() {}

   protected:
    // --- Behaviour context ------------------------------------------------------------------
    // A handle to the entity this script drives (its id bound to its registry). Everything below
    // is sugar over it; use it directly for the full EntityHandle surface (add/remove/has/valid).
    EntityHandle self() const { return EntityHandle{m_entity, m_registry}; }

    // This entity's transform, or nullptr if it has none: transform()->setPosition(...).
    TransformComponent* transform() const { return self().transform(); }

    // Any component on this entity, or nullptr if absent: getComponent<RenderComponent>().
    template<typename T>
    T* getComponent() const {
        return self().get<T>();
    }

    // The scene this script lives in (nullptr only if it runs outside an active scene).
    Scene* scene() const { return m_scene; }

    // The engine input service, or nullptr if none is wired: input()->isKeyDown(Key::KEY_W).
    InputManager* input() const { return m_input; }

    // Seconds elapsed since scripting started running (accumulated frame deltas). The same value
    // for every script within a frame.
    double time() const { return m_time; }

    // Spawn another entity into this script's scene and return a handle to its root. Forwards to
    // Scene::spawn (a model id today; Prefab support lands in T7). A template so a script only
    // needs Scene complete (include <Scene.h>) when it actually instantiates.
    template<typename... Args>
    EntityHandle instantiate(Args&&... args) {
        return scene()->spawn(std::forward<Args>(args)...);
    }

    // Mark this entity for destruction. Teardown (onDestroy + component/entity removal) is deferred
    // to the end of the current ScriptSystem update, so this is safe to call from within onUpdate:
    // the running script and its components stay valid until the frame's script pass finishes.
    void destroy() { m_destroyed = true; }

    // Retained lower-level accessors (self()/transform()/getComponent<T>() supersede these; kept
    // for existing scripts and the rare case that wants the raw id / registry).
    Entity entity() const { return m_entity; }
    Registry* registry() const { return m_registry; }

   private:
    // The ScriptSystem is the sole injector of this context and the reader of m_destroyed.
    friend class ScriptSystem;
    Entity m_entity = NULL_ENTITY;
    Registry* m_registry = nullptr;
    Scene* m_scene = nullptr;
    InputManager* m_input = nullptr;
    double m_time = 0.0;
    bool m_destroyed = false;
};
}  // namespace ICE
