//
// Created by Thomas Ibanez on 26.01.23.
//

#ifndef ICE_REGISTRY_H
#define ICE_REGISTRY_H

#include <AnimationComponent.h>
#include <CameraComponent.h>
#include <Component.h>
#include <Entity.h>
#include <LightComponent.h>
#include <RenderComponent.h>
#include <RenderSystem.h>
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
        componentManager.registerComponent<CameraComponent>();
        componentManager.registerComponent<SkyboxComponent>();
        componentManager.registerComponent<AnimationComponent>();
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
        entities.erase(it);
        componentManager.entityDestroyed(e);
        entityManager.releaseEntity(e);
        systemManager.entityDestroyed(e);
    }

    std::vector<Entity> getEntities() const { return entities; }

    template<typename T>
    bool entityHasComponent(Entity e) {
        return entityManager.getSignature(e).test(componentManager.getComponentType<T>());
    }

    template<typename T>
    T *getComponent(Entity e) {
        return componentManager.getComponent<T>(e);
    }

    template<typename T>
    void addComponent(Entity e, T component) {
        componentManager.addComponent<T>(e, component);
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
