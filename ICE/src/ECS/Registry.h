//
// Created by Thomas Ibanez on 26.01.23.
//

#ifndef ICE_REGISTRY_H
#define ICE_REGISTRY_H

#include <ECS/Entity.h>
#include <ECS/Component.h>
#include <vector>

namespace ICE {
    class Registry
    {
    private:
        EntityManager entityManager;
        ComponentManager componentManager;
        std::vector<Entity> entities;
    public:
        Registry(/* args */) {};
        ~Registry() {};

        Entity createEntity() {
            Entity e = entityManager.createEntity();
            entities.push_back(e);
            return e;
        }

        void removeEntity(Entity e) {
            auto it = std::find(entities.begin(), entities.end(), e);
            entities.erase(it);
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
        }

        template <typename T>
        void removeComponent(Entity e) {
            componentManager.removeComponent<T>(e);
            auto signature = entityManager.getSignature(e);
            signature.set(componentManager.getComponentType<T>(), false);
            entityManager.setSignature(e, signature);
        }
    };
}

#endif //ICE_REGISTRY_H
