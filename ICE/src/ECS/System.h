//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_SYSTEM_H
#define ICE_SYSTEM_H

#include <Scene/Scene.h>
#include <set>
#include <typeindex>

namespace ICE {
    class System {
    public:
        std::set<Entity> entities;
        virtual void update(Scene* scene, double delta) = 0;
        virtual void entitySignatureChanged(Entity e) = 0;
    };

    class SystemManager {
    public:
        template<typename T>
        std::shared_ptr<T> registerSystem() {
            assert(systems.find(typeid(T)) == systems.end() && "Registering system more than once.");

            // Create a pointer to the system and return it so it can be used externally
            auto system = std::make_shared<T>();
            systems.insert({typeid(T), system});
            return system;
        }

        template<typename T>
        void setSignature(Signature signature) {
            assert(systems.find(typeid(T)) != systems.end() && "System used before registered.");

            // Set the signature for this system
            signatures.insert({typeid(T), signature});
        }

        void entityDestroyed(Entity entity) {
            // Erase a destroyed entity from all system lists
            // mEntities is a set so no check needed
            for (auto const& pair : systems)
            {
                auto const& system = pair.second;

                system->entities.erase(entity);
            }
        }

        void entitySignatureChanged(Entity entity, Signature entitySignature) {
            // Notify each system that an entity's signature changed
            for (auto const& pair : systems)
            {
                auto const& system = pair.second;
                auto const& systemSignature = signatures[pair.first];

                // Entity signature matches system signature - insert into set
                if ((entitySignature & systemSignature) == systemSignature)
                {
                    system->entities.insert(entity);
                }
                // Entity signature does not match system signature - erase from set
                else
                {
                    system->entities.erase(entity);
                }
            }
        }

    private:
        // Map from system type string pointer to a signature
        std::unordered_map<std::type_index, Signature> signatures{};

        // Map from system type string pointer to a system pointer
        std::unordered_map<std::type_index, std::shared_ptr<System>> systems{};

    };
}

#endif //ICE_SYSTEM_H
