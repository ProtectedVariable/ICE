#pragma once

#include <AnimationComponent.h>
#include <LightComponent.h>
#include <Registry.h>
#include <RenderComponent.h>
#include <TransformComponent.h>

#include <utility>

namespace ICE {

// Lightweight value-type handle over an entity: bundles the entity id with its registry so
// component access reads naturally and liveness is checkable. Cheap to copy (an id plus a
// pointer) and owns nothing. Implicitly decays to the raw Entity id, so a handle drops into
// the existing id-based APIs (scene graph, picking, serialization, ...) unchanged.
//
//   EntityHandle e = scene->create("Player");
//   e.add(TransformComponent(...));
//   e.transform()->setPosition({0, 1, 0});
//   if (e.has<LightComponent>()) { ... }
//   if (!e.valid()) { ... }                 // entity destroyed since we took the handle
//
// This supersedes the old EntityHelper. Component pointers returned here follow the same validity
// guarantee as Registry::getComponent: they stay valid until that component is removed, and are
// not invalidated by add/remove of any other entity's components.
class EntityHandle {
   public:
    EntityHandle() = default;
    EntityHandle(Entity id, Registry* registry) : m_id(id), m_registry(registry) {}

    // --- Component access -------------------------------------------------------------------
    // Add a component (moved in) and return a reference to the stored instance.
    template<typename T>
    T& add(T component) {
        m_registry->addComponent<T>(m_id, std::move(component));
        return *m_registry->getComponent<T>(m_id);
    }

    template<typename T>
    void remove() {
        m_registry->removeComponent<T>(m_id);
    }

    // nullptr if the entity has no component of type T (also safe on a stale/destroyed handle).
    template<typename T>
    T* get() const {
        return m_registry->tryGetComponent<T>(m_id);
    }

    template<typename T>
    bool has() const {
        return get<T>() != nullptr;
    }

    // Convenience accessors for the built-in components (nullptr if absent).
    TransformComponent* transform() const { return get<TransformComponent>(); }
    RenderComponent* render() const { return get<RenderComponent>(); }
    LightComponent* light() const { return get<LightComponent>(); }
    AnimationComponent* animation() const { return get<AnimationComponent>(); }

    // --- Identity / liveness ----------------------------------------------------------------
    Entity id() const { return m_id; }
    Registry* registry() const { return m_registry; }

    // True while the entity is live in its registry (Phase 3 liveness). A default/null handle
    // and a handle to a destroyed entity are both invalid.
    bool valid() const { return m_registry != nullptr && m_registry->isAlive(m_id); }
    explicit operator bool() const { return valid(); }

    // Implicit decay to the raw id for interop with id-based APIs.
    operator Entity() const { return m_id; }

    bool operator==(const EntityHandle& o) const { return m_id == o.m_id && m_registry == o.m_registry; }
    bool operator!=(const EntityHandle& o) const { return !(*this == o); }

   private:
    Entity m_id = NULL_ENTITY;
    Registry* m_registry = nullptr;
};
}  // namespace ICE
