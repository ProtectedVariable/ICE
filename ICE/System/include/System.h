//
// Created by Thomas Ibanez on 19.11.20.
//

#pragma once

#include <Entity.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_set>
#include <vector>

namespace ICE {
class Scene;
class ComponentManager;

// Canonical per-frame update order (lower runs first): input/scripts advance state,
// animation updates local transforms, the scene graph propagates them to world space,
// then rendering consumes the final transforms.
enum SystemUpdateOrder : int {
    ScriptSystemOrder = 100,
    AnimationSystemOrder = 200,
    SceneGraphSystemOrder = 300,
    RenderSystemOrder = 400,
};

class System {
   public:
    virtual void update(double delta) = 0;
    virtual void onEntityAdded(Entity e) {};
    virtual void onEntityRemoved(Entity e) {};

    // Lower values update first. Determines the deterministic per-frame ordering; the
    // default puts unclassified systems before rendering.
    virtual int updateOrder() const { return 0; }

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
        for (auto const& system : orderedSystems) {
            system->entities.erase(entity);
            system->onEntityRemoved(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature entitySignature, const ComponentManager& comp_manager) {
        // Notify each system that an entity's signature changed
        for (auto const& system : orderedSystems) {
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
        // Iterate in deterministic updateOrder() order (not hash order of the type map).
        for (auto const& system : orderedSystems) {
            system->update(delta);
        }
    }

    template<typename T>
    void addSystem(const std::shared_ptr<T>& system) {
        if (!systems.try_emplace(typeid(T), system).second) {
            return;  // already registered
        }
        // Keep orderedSystems sorted by updateOrder(); stable for equal orders (later
        // registrations of the same order run after earlier ones).
        auto pos = std::upper_bound(orderedSystems.begin(), orderedSystems.end(), std::static_pointer_cast<System>(system),
                                    [](const std::shared_ptr<System>& a, const std::shared_ptr<System>& b) { return a->updateOrder() < b->updateOrder(); });
        orderedSystems.insert(pos, system);
    }

    template<typename T>
    std::shared_ptr<T> getSystem() {
        return std::static_pointer_cast<T>(systems.at(typeid(T)));
    }

   private:
    // Map from system type index to a system pointer (fast getSystem<T>() lookup).
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
    // Same systems, kept sorted by updateOrder() for deterministic iteration.
    std::vector<std::shared_ptr<System>> orderedSystems;
};
}  // namespace ICE
