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
            // A system's signatures are fixed once component types are registered, so cache
            // them instead of rebuilding the vector on every component add/remove.
            auto sig_it = m_signatureCache.find(system.get());
            if (sig_it == m_signatureCache.end()) {
                sig_it = m_signatureCache.emplace(system.get(), system->getSignatures(comp_manager)).first;
            }
            const auto& systemSignature = sig_it->second;

            // Entity signature matches system signature - insert into set. onEntityAdded is
            // fired on every matching signature change (not just the first insert): a system
            // like RenderSystem derives several sub-lists (renderables/lights/skybox) from
            // different component types and must resync when any relevant component is added
            // or removed while the entity is already a member. Such onEntityAdded handlers
            // must therefore be idempotent (resync, e.g. remove-then-add their sub-lists).
            bool match = false;
            for (const auto& s : systemSignature) {
                if ((entitySignature & s) == s) {
                    system->entities.insert(entity);
                    system->onEntityAdded(entity);
                    match = true;
                    break;
                }
            }
            // Entity signature no longer matches - erase from set and notify (both idempotent).
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

    // Non-throwing variant: nullptr if no system of type T has been added (getSystem throws).
    template<typename T>
    std::shared_ptr<T> tryGetSystem() {
        auto it = systems.find(typeid(T));
        return it == systems.end() ? nullptr : std::static_pointer_cast<T>(it->second);
    }

   private:
    // Map from system type index to a system pointer (fast getSystem<T>() lookup).
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
    // Same systems, kept sorted by updateOrder() for deterministic iteration.
    std::vector<std::shared_ptr<System>> orderedSystems;
    // Cached signatures per system (stable after component registration); avoids rebuilding
    // the vector on every entitySignatureChanged call.
    std::unordered_map<System*, std::vector<Signature>> m_signatureCache;
};
}  // namespace ICE
