//
// Created by Thomas Ibanez on 26.01.23.
//

#ifndef ICE_REGISTRY_H
#define ICE_REGISTRY_H

#include <Component.h>
#include <Entity.h>
#include <System.h>

#include <vector>

namespace ICE {
// Component types are no longer enumerated here: they register themselves the first time they are
// added/queried (see ComponentManager::assure). This keeps the ECS core free of any dependency on
// concrete gameplay/render component headers -- add a brand-new component type from anywhere with
// no registration call and no edit to this file.
class Registry {
   public:
    Registry() = default;
    ~Registry() = default;

    // Deprecated: component types register on first use, so this call is unnecessary. Kept as an
    // idempotent forwarder until existing call sites are removed.
    template<typename T>
    [[deprecated("Component types register on first use; registerCustomComponent() is no longer required.")]]
    void registerCustomComponent() {
        componentManager.assure<T>();
    }

    Entity createEntity() {
        Entity e = entityManager.createEntity();
        entities.push_back(e);
        return e;
    }

    void addEntity(Entity e) {
        entityManager.createEntity(e);
        entities.push_back(e);
    }

    void removeEntity(Entity e) {
        auto it = std::find(entities.begin(), entities.end(), e);
        if (it == entities.end()) {
            // Not a live entity: erase(end()) would be undefined behavior.
            return;
        }
        entities.erase(it);
        componentManager.entityDestroyed(e);
        entityManager.releaseEntity(e);
        systemManager.entityDestroyed(e);
    }

    const std::vector<Entity>& getEntities() const { return entities; }

    bool isAlive(Entity e) const { return entityManager.isAlive(e); }

    template<typename T>
    bool entityHasComponent(Entity e) const {
        return entityManager.getSignature(e).test(componentManager.getComponentType<T>());
    }

    // The returned pointer stays valid until *this* component is removed (removeComponent<T>(e)
    // or the entity/registry is destroyed). Component storage is stable: adding or removing any
    // OTHER entity's component -- of this or any other type -- never invalidates it, so it is safe
    // to hold across add/remove of other entities. Removing this component leaves the pointer
    // dangling, as with erasing any container element.
    template<typename T>
    T *getComponent(Entity e) {
        return componentManager.getComponent<T>(e);
    }

    // Returns nullptr instead of asserting when the entity has no component of type T. Same
    // pointer-validity guarantee as getComponent.
    template<typename T>
    T *tryGetComponent(Entity e) {
        return componentManager.tryGetComponent<T>(e);
    }

    // Cache-friendly iteration over all entities that have every listed component type.
    // Iterates the first type's dense storage, so list the rarest component first:
    //   registry.each<LightComponent, TransformComponent>([](Entity e, LightComponent& l, TransformComponent& t){ ... });
    // Don't add/remove any of these component types from within the callback.
    template<typename... Ts, typename Fn>
    void each(Fn &&fn) {
        componentManager.each<Ts...>(std::forward<Fn>(fn));
    }

    template<typename T>
    void addComponent(Entity e, T component) {
        componentManager.addComponent<T>(e, std::move(component));
        auto signature = entityManager.getSignature(e);
        signature.set(componentManager.getComponentType<T>(), true);
        entityManager.setSignature(e, signature);
        systemManager.entitySignatureChanged(e, signature, componentManager);
    }

    template<typename T>
    void removeComponent(Entity e) {
        componentManager.removeComponent<T>(e);
        auto signature = entityManager.getSignature(e);
        signature.set(componentManager.getComponentType<T>(), false);
        entityManager.setSignature(e, signature);
        systemManager.entitySignatureChanged(e, signature, componentManager);
    }

    template<typename T>
    void addSystem(const std::shared_ptr<T> &system) {
        systemManager.addSystem(system);
        for (const auto e : entities) {
            systemManager.entitySignatureChanged(e, entityManager.getSignature(e), componentManager);
        }
    }

    template<typename T>
    std::shared_ptr<T> getSystem() {
        return systemManager.getSystem<T>();
    }

    // nullptr if no system of type T has been added (getSystem throws in that case).
    template<typename T>
    std::shared_ptr<T> tryGetSystem() {
        return systemManager.tryGetSystem<T>();
    }

    void updateSystems(double delta) { systemManager.updateSystems(delta); }

   private:
    EntityManager entityManager;
    ComponentManager componentManager;
    SystemManager systemManager;
    std::vector<Entity> entities;
};
}  // namespace ICE

#endif  //ICE_REGISTRY_H
