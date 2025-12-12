//
// Created by Thomas Ibanez on 19.11.20.
//

#pragma once

#include <Entity.h>

#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_set>
#include <vector>

namespace ICE {
class Scene;
class ComponentManager;

class System {
   public:
    virtual void update(double delta) = 0;
    virtual void onEntityAdded(Entity e) {};
    virtual void onEntityRemoved(Entity e) {};

    virtual std::vector<Signature> getSignatures(const ComponentManager& comp_manager) const = 0;
    virtual ~System() = default;

   protected:
    std::unordered_set<Entity> entities;

    friend class SystemManager;
};

class SystemManager {
   public:
    void entityDestroyed(Entity entity) {
        // Erase a destroyed entity from all system lists
        // mEntities is a set so no check needed
        for (auto const& pair : systems) {
            auto const& system = pair.second;

            system->entities.erase(entity);
            system->onEntityRemoved(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature entitySignature, const ComponentManager& comp_manager) {
        // Notify each system that an entity's signature changed
        for (auto const& pair : systems) {
            auto const& system = pair.second;
            auto const& systemSignature = system->getSignatures(comp_manager);

            // Entity signature matches system signature - insert into set
            bool match = false;
            for (const auto& s : systemSignature) {
                if ((entitySignature & s) == s) {
                    system->entities.insert(entity);
                    system->onEntityAdded(entity);
                    match = true;
                    break;
                }
            }
            // Entity signature does not match system signature - erase from set
            if (!match) {
                system->entities.erase(entity);
                system->onEntityRemoved(entity);
            }
        }
    }

    void updateSystems(double delta) {
        for (auto const& [signature, system] : systems) {
            system->update(delta);
        }
    }

    template<typename T>
    void addSystem(const std::shared_ptr<T>& system) {
        systems.try_emplace(typeid(T), system);
    }

    template<typename T>
    std::shared_ptr<T> getSystem() {
        return std::static_pointer_cast<T>(systems.at(typeid(T)));
    }

   private:
    // Map from system type string pointer to a system pointer
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
};
}  // namespace ICE
