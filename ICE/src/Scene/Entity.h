//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_ENTITY_H
#define ICE_ENTITY_H

#include <cstdlib>
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
            auto f = this->components.find(typeid(T));
            return f != this->components.end();
        }

        template<typename T>
        bool addComponent(T* component) {
            this->components[typeid(T)] = component;
            return true;
        }
        template<typename T>
        bool removeComponent() {
            return this->components.erase(typeid(T));
        }

    private:
        std::unordered_map<std::type_index, Component*> components;
    };
}

#endif //ICE_ENTITY_H
