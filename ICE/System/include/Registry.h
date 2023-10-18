//
// Created by Thomas Ibanez on 26.01.23.
//

#ifndef ICE_REGISTRY_H
#define ICE_REGISTRY_H

#include <ECS/Entity.h>
#include <ECS/Component.h>
#include <vector>
#include <ECS/CameraComponent.h>
#include <ECS/TransformComponent.h>
#include <ECS/RenderComponent.h>
#include <ECS/LightComponent.h>
#include <ECS/System.h>
#include <ECS/RenderSystem.h>

namespace ICE {
    class Registry
    {
    public:
        Registry(/* args */) {
            componentManager.registerComponent<TransformComponent>();
            componentManager.registerComponent<RenderComponent>();
            componentManager.registerComponent<LightComponent>();
            componentManager.registerComponent<CameraComponent>();

            Signature signature;
            signature.set(componentManager.getComponentType<RenderComponent>());
            signature.set(componentManager.getComponentType<TransformComponent>());
            systemManager.registerSystem<RenderSystem>();
            systemManager.addSignature<RenderSystem>(signature);
            signature.set(componentManager.getComponentType<RenderComponent>(), false);
            signature.set(componentManager.getComponentType<LightComponent>());
            systemManager.addSignature<RenderSystem>(signature);
            signature.reset();
        }
        ~Registry() {};

        Entity createEntity() {
            Entity e = entityManager.createEntity();
            entities.push_back(e);
            return e;
        }

        void removeEntity(Entity e) {
            auto it = std::find(entities.begin(), entities.end(), e);
            entities.erase(it);
            entityManager.releaseEntity(e);
            systemManager.entityDestroyed(e);
        }

        std::vector<Entity> getEntities() const {
            return entities;
        }

        template <typename T>
        bool entityHasComponent(Entity e) {
            return entityManager.getSignature(e).test(componentManager.getComponentType<T>());
        }

        template <typename T>
        T* getComponent(Entity e) {
            return componentManager.getComponent<T>(e);
        }

        template <typename T>
        void addComponent(Entity e, T component) {
            componentManager.addComponent<T>(e, component);
            auto signature = entityManager.getSignature(e);
            signature.set(componentManager.getComponentType<T>(), true);
            entityManager.setSignature(e, signature);
            systemManager.entitySignatureChanged(e, signature);
        }

        template <typename T>
        void removeComponent(Entity e) {
            componentManager.removeComponent<T>(e);
            auto signature = entityManager.getSignature(e);
            signature.set(componentManager.getComponentType<T>(), false);
            entityManager.setSignature(e, signature);
            systemManager.entitySignatureChanged(e, signature);
        }

    private:
        EntityManager entityManager;
        ComponentManager componentManager;
        SystemManager systemManager;
        std::vector<Entity> entities;
    };
}

#endif //ICE_REGISTRY_H
