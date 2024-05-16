//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_SYSTEM_H
#define ICE_SYSTEM_H

#include <Entity.h>

#include <memory>
#include <set>
#include <typeindex>
#include <vector>
#include <cassert>

namespace ICE {
class Scene;

class System {
   public:
    std::set<Entity> entities;
    virtual void update(double delta) = 0;
};

class SystemManager {
   public:
    template<typename T>
    void registerSystem() {
        assert(systems.find(typeid(T)) == systems.end() && "Registering system more than once.");
        signatures.insert({typeid(T), std::vector<Signature>()});
    }

    template<typename T>
    void addSignature(Signature signature) {
        assert(signatures.find(typeid(T)) != signatures.end() && "System used before registered.");

        // Set the signature for this system
        signatures[typeid(T)].push_back(signature);
    }

    void entityDestroyed(Entity entity) {
        // Erase a destroyed entity from all system lists
        // mEntities is a set so no check needed
        for (auto const& pair : systems) {
            auto const& system = pair.second;

            system->entities.erase(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature entitySignature) {
        // Notify each system that an entity's signature changed
        for (auto const& pair : systems) {
            auto const& system = pair.second;
            auto const& systemSignature = signatures[pair.first];

            // Entity signature matches system signature - insert into set
            bool match = false;
            for (const auto& s : systemSignature) {
                if ((entitySignature & s) == s) {
                    system->entities.insert(entity);
                    match = true;
                    break;
                }
            }
            // Entity signature does not match system signature - erase from set
            if (!match) {
                system->entities.erase(entity);
            }
        }
    }

    void updateSystems(double delta) {
        for (auto const& [signature, system] : systems) {
            system->update(delta);
        }
    }

    template <typename T>
    void addSystem(const std::shared_ptr<T> &system) {
        systems.try_emplace(typeid(T), system);
    }

    template<typename T>
    std::shared_ptr<T> getSystem() {
        return std::static_pointer_cast<T>(systems.at(typeid(T)));
    }

   private:
    // Map from system type string pointer to a signature
    std::unordered_map<std::type_index, std::vector<Signature>> signatures;

    // Map from system type string pointer to a system pointer
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
};
}  // namespace ICE

#endif  //ICE_SYSTEM_H
