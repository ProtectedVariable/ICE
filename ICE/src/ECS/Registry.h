//
// Created by Thomas Ibanez on 26.01.23.
//

#ifndef ICE_REGISTRY_H
#define ICE_REGISTRY_H

#include <ECS/Entity.h>
#include <ECS/Component.h>

namespace ICE {
    class Registry
    {
    private:
        EntityManager entityManager;
        ComponentManager componentManager;

    public:
        Registry(/* args */) {};
        ~Registry() {};

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
    


#endif //ICE_REGISTRY_H
