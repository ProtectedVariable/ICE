//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_ENTITY_H
#define ICE_ENTITY_H

#include <stdlib.h>
#include <unordered_map>
#include <Scene/Component.h>
#include <typeindex>

namespace ICE {
    class Entity {
    public:
        template<typename T>
        T* getComponent() {
            return static_cast<T*>(this->components[typeid(T)]);
        }

        template<typename T>
        bool hasComponent() {
            return this->components.find(typeid(T)) != this->components.end();
        }

        template<typename T>
        bool addComponent(T &component) {
            this->components[typeid(component)] = &component;
            return true;
        }

    private:
        std::unordered_map<std::type_index, Component*> components;
    };
}

#endif //ICE_ENTITY_H
