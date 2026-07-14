//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_COMPONENT_H
#define ICE_COMPONENT_H

#include <Entity.h>

#include <cassert>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace ICE {

using ComponentType = std::uint8_t;

struct Component {};

class IComponentArray {
   public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

// Sparse set with STABLE component storage. Components live in a std::deque pool whose element
// addresses never move: growth appends without relocating existing elements, and removal frees a
// slot (recorded in freeSlots) without moving any survivor. Consequently a T* returned by
// getData/tryGetData stays valid until *that* component is removed -- adding or removing any OTHER
// entity's component of this type never invalidates it. (The previous std::vector backing could
// reallocate on insert or swap-move the tail element on erase, invalidating outstanding pointers;
// this is the hazard the WARNING comments on Registry::getComponent used to describe.)
//
// Contract: a component pointer is valid until that component is removed (or the array is
// destroyed). A freed slot is later reused by an insert, so a pointer to an already-removed
// component may then alias a different entity's component -- but that pointer was dangling by
// contract the moment its component was removed, exactly as erasing an element invalidates
// pointers to it in any container.
//
// The public surface (insertData/removeData/tryGetData/getData/count/forEach/entityDestroyed) is
// unchanged. Lookups index the sparse array directly (no hashing); iteration walks the pool
// skipping freed slots.
template<typename T>
class ComponentArray : public IComponentArray {
   public:
    void insertData(Entity entity, T component) {
        assert(!has(entity) && "Component added to same entity more than once.");

        // Grow the sparse array so `entity` is addressable, filling gaps with TOMBSTONE.
        if (entity >= sparse.size()) {
            sparse.resize(entity + 1, TOMBSTONE);
        }

        uint32_t slot;
        if (!freeSlots.empty()) {
            // Reuse a hole left by a previous removal; the slot's address is unchanged, so a
            // pointer to any other live component is unaffected.
            slot = freeSlots.back();
            freeSlots.pop_back();
            pool[slot] = std::move(component);
            slotToEntity[slot] = entity;
        } else {
            // Append a fresh slot. std::deque::push_back keeps references to existing elements
            // valid (no relocation), so outstanding component pointers survive the growth.
            slot = static_cast<uint32_t>(pool.size());
            pool.push_back(std::move(component));
            slotToEntity.push_back(entity);
        }
        sparse[entity] = slot;
        ++liveCount;
    }

    void removeData(Entity entity) {
        assert(has(entity) && "Removing non-existent component.");

        // Tombstone the slot and return it to the free list. Survivors are NOT moved, so every
        // other component's address is preserved. For components that own resources (strings,
        // buffers, std::function, ...), move the removed one out into a temporary so those
        // resources are freed now rather than lingering until the slot is reused; the slot itself
        // stays put (left moved-from but alive). Trivially-destructible components own nothing, so
        // skip the move entirely.
        uint32_t slot = sparse[entity];
        if constexpr (!std::is_trivially_destructible_v<T>) {
            T released = std::move(pool[slot]);
        }
        slotToEntity[slot] = TOMBSTONE;
        freeSlots.push_back(slot);
        sparse[entity] = TOMBSTONE;
        --liveCount;
    }

    // Returns nullptr (instead of silently aliasing another entity's slot) when the
    // entity has no component of this type. A missing entity is any id outside the sparse
    // array or one whose sparse slot is TOMBSTONE. Callers that require the component should
    // use getData and check the result.
    T* tryGetData(Entity entity) {
        if (!has(entity)) {
            return nullptr;
        }
        return &pool[sparse[entity]];
    }

    T* getData(Entity entity) {
        T* data = tryGetData(entity);
        assert(data != nullptr && "getData: entity has no component of this type.");
        return data;
    }

    size_t count() const { return liveCount; }

    // Iterate the live components (no per-entity hash lookup), invoking fn(Entity, T&) for each.
    // Walks the pool in slot order, skipping the holes left by removals; with low churn there are
    // few holes to skip. Do not add/remove components of type T from within the callback (the set
    // of visited slots is captured by the loop bound).
    template<typename Fn>
    void forEach(Fn&& fn) {
        for (size_t slot = 0; slot < pool.size(); ++slot) {
            Entity e = slotToEntity[slot];
            if (e != TOMBSTONE) {
                fn(e, pool[slot]);
            }
        }
    }

    void entityDestroyed(Entity entity) override {
        if (has(entity)) {
            // Remove the entity's component if it existed
            removeData(entity);
        }
    }

   private:
    // Sentinel: in `sparse` marks an entity with no component of type T; in `slotToEntity` marks
    // a free (tombstoned) pool slot. Entity ids never reach this value in practice.
    static constexpr uint32_t TOMBSTONE = std::numeric_limits<uint32_t>::max();

    // True if `entity` currently owns a component.
    bool has(Entity entity) const {
        return entity < sparse.size() && sparse[entity] != TOMBSTONE;
    }

    // Stable component pool: a slot's address never changes for the component's lifetime. Never
    // erased from directly -- removals tombstone the slot and push it to freeSlots for reuse, so
    // the pool grows only to the high-water mark of simultaneously-live components.
    std::deque<T> pool;

    // Owner entity per pool slot, or TOMBSTONE for a free slot. Parallel to `pool`; also the
    // liveness marker walked by forEach.
    std::vector<Entity> slotToEntity;

    // sparse[entity] is the pool slot for that entity, or TOMBSTONE if absent. Indexed directly
    // by entity id (grown on demand), so lookups never hash.
    std::vector<uint32_t> sparse;

    // Slots freed by removals, available for reuse by the next insert.
    std::vector<uint32_t> freeSlots;

    // Number of live components (pool.size() minus the current holes).
    size_t liveCount = 0;
};

// Assigns a stable id to each component type the first time it is queried, entt-style. Ids are
// process-wide (independent of any ComponentManager instance) and, crucially, independent of
// whether the component's storage exists yet -- so getComponentType<T>() is valid in const
// contexts such as System::getSignatures even before the first component of type T is ever
// added. Ids are handed out in first-touch order across the whole process; nothing persists them
// (scenes serialize component *data*, not type ids), so the order only needs to be stable within
// a single run.
//
// NOTE (threading): id<T>()'s local static is initialised once under the C++ magic-static guard,
// but the shared `counter` increment inside next() is not atomic. Registration is single-threaded
// today; when the job scheduler lands, first-touch of a new component type must be serialised
// (pre-warm all types, or guard next()).
class ComponentTypeRegistry {
   public:
    template<typename T>
    static ComponentType id() {
        static const ComponentType value = next();
        return value;
    }

   private:
    static ComponentType next() {
        static ComponentType counter = 0;
        // A type id must be a valid Signature bit index; index == MaxComponentTypes would throw
        // from Signature::set() at runtime.
        assert(counter < StoragePolicy::MaxComponentTypes && "Too many distinct component types registered.");
        return counter++;
    }
};

class ComponentManager {
   public:
    // Ensures the storage for component type T exists, creating it on first use, and returns it.
    // This is the single lazy-registration entry point: adding, getting or iterating a component
    // type routes through here, so there is no separate registration step. Idempotent.
    template<typename T>
    ComponentArray<T>& assure() {
        auto const& type = typeid(T);
        auto it = componentArrays.find(type);
        if (it == componentArrays.end()) {
            it = componentArrays.emplace(type, std::make_shared<ComponentArray<T>>()).first;
        }
        return static_cast<ComponentArray<T>&>(*it->second);
    }

    // Deprecated: component types now register lazily on first use. Kept as an idempotent
    // forwarder so existing explicit-registration call sites keep compiling.
    template<typename T>
    [[deprecated("Component types register on first use; registerComponent() is no longer required.")]]
    void registerComponent() {
        assure<T>();
    }

    // Stable signature-bit index for component type T. Const and side-effect-free on the manager
    // (the id comes from the process-wide ComponentTypeRegistry), so it works before T has any
    // storage -- required by the const System::getSignatures path.
    template<typename T>
    ComponentType getComponentType() const {
        return ComponentTypeRegistry::id<T>();
    }

    template<typename T>
    void addComponent(Entity entity, T component) {
        // Add a component to the array for an entity; move to avoid an extra copy of
        // potentially heavy components. Registers the type on first use.
        assure<T>().insertData(entity, std::move(component));
    }

    template<typename T>
    void removeComponent(Entity entity) {
        assure<T>().removeData(entity);
    }

    template<typename T>
    T* getComponent(Entity entity) {
        // Get a reference to a component from the array for an entity
        return assure<T>().getData(entity);
    }

    // Returns nullptr if the type is not registered or the entity has no such component,
    // instead of asserting/throwing. For callers that legitimately probe for a component.
    template<typename T>
    T* tryGetComponent(Entity entity) {
        auto* arr = tryGetComponentArrayPtr<T>();
        return arr == nullptr ? nullptr : arr->tryGetData(entity);
    }

    // View iteration: invokes fn(Entity, Primary&, Others&...) for every entity that has all
    // of the listed component types. Walks Primary's storage pool and probes the others by
    // lookup, so list the rarest component first. Behaves like a minimal entt-style view. Do not
    // add/remove any of these component types inside fn.
    template<typename Primary, typename... Others, typename Fn>
    void each(Fn&& fn) {
        auto* primaryArr = tryGetComponentArrayPtr<Primary>();
        if (primaryArr == nullptr) {
            return;
        }
        if constexpr (sizeof...(Others) > 0) {
            if (((tryGetComponentArrayPtr<Others>() == nullptr) || ...)) {
                return;  // an "Others" type isn't registered, so nothing can match
            }
        }
        primaryArr->forEach([&](Entity e, Primary& primary) {
            if constexpr (sizeof...(Others) == 0) {
                fn(e, primary);
            } else {
                std::tuple<Others*...> others{tryGetComponent<Others>(e)...};
                const bool all_present = ((std::get<Others*>(others) != nullptr) && ...);
                if (all_present) {
                    fn(e, primary, *std::get<Others*>(others)...);
                }
            }
        });
    }

    void entityDestroyed(Entity entity) {
        // Notify each component array that an entity has been destroyed
        // If it has a component for that entity, it will remove it
        for (auto const& pair : componentArrays) {
            auto const& component = pair.second;

            component->entityDestroyed(entity);
        }
    }

   private:
    // Lazily-created component storage, keyed by type. An entry appears the first time a
    // component of that type is touched (see assure); type ids for signatures come separately
    // from ComponentTypeRegistry, so this map no longer tracks them.
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> componentArrays{};

    // Non-throwing storage lookup: nullptr if no component of type T has ever been created.
    // Used by the probe/iteration paths (tryGetComponent/each) so they never lazily register a
    // type just by looking for it.
    template<typename T>
    ComponentArray<T>* tryGetComponentArrayPtr() {
        auto it = componentArrays.find(typeid(T));
        return it == componentArrays.end() ? nullptr : static_cast<ComponentArray<T>*>(it->second.get());
    }
};
}  // namespace ICE

#endif  //ICE_COMPONENT_H
