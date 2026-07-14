//
// Created by Thomas Ibanez on 26.01.23.
//

#ifndef ICE_REGISTRY_H
#define ICE_REGISTRY_H

#include <AnimationComponent.h>
#include <Component.h>
#include <Entity.h>
#include <LightComponent.h>
#include <NativeScriptComponent.h>
#include <RenderComponent.h>
#include <SkeletonPoseComponent.h>
#include <SkinningComponent.h>
#include <SkyboxComponent.h>
#include <System.h>
#include <TransformComponent.h>

#include <vector>

namespace ICE {
class Registry {
   public:
    Registry() {
        componentManager.registerComponent<TransformComponent>();
        componentManager.registerComponent<RenderComponent>();
        componentManager.registerComponent<LightComponent>();
        componentManager.registerComponent<SkyboxComponent>();
        componentManager.registerComponent<AnimationComponent>();
        componentManager.registerComponent<SkeletonPoseComponent>();
        componentManager.registerComponent<SkinningComponent>();
        componentManager.registerComponent<NativeScriptComponent>();
    }
    ~Registry() = default;

    template<typename T>
    void registerCustomComponent() {
        componentManager.registerComponent<T>();
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

    // WARNING: the returned pointer is only valid until the next structural change to the
    // T component storage. addComponent<T>/removeComponent<T> on ANY entity can reallocate
    // or swap-move the backing vector, invalidating outstanding T* handles. Do not cache
    // component pointers across such changes -- re-fetch instead (see the editor Inspector).
    template<typename T>
    T *getComponent(Entity e) {
        return componentManager.getComponent<T>(e);
    }

    // Returns nullptr instead of asserting when the entity has no component of type T. Same
    // pointer-invalidation caveat as getComponent applies.
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

    void updateSystems(double delta) { systemManager.updateSystems(delta); }

   private:
    EntityManager entityManager;
    ComponentManager componentManager;
    SystemManager systemManager;
    std::vector<Entity> entities;
};
}  // namespace ICE

#endif  //ICE_REGISTRY_H
