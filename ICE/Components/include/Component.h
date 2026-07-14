//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_COMPONENT_H
#define ICE_COMPONENT_H

#include <Entity.h>

#include <cassert>
#include <memory>
#include <tuple>
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

template<typename T>
class ComponentArray : public IComponentArray {
   public:
    void insertData(Entity entity, T component) {
        assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to same entity more than once.");

        // Put new entry at end and update the maps
        size_t newIndex = size;
        entityToIndexMap[entity] = newIndex;
        indexToEntityMap[newIndex] = entity;
        if (size < componentArray.size()) {
            componentArray[newIndex] = std::move(component);
        } else {
            componentArray.push_back(std::move(component));
        }
        size++;
    }

    void removeData(Entity entity) {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Removing non-existent component.");

        // Copy element at end into deleted element's place to maintain density
        size_t indexOfRemovedEntity = entityToIndexMap[entity];
        size_t indexOfLastElement = size - 1;
        componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

        // Update map to point to moved spot
        Entity entityOfLastElement = indexToEntityMap[indexOfLastElement];
        entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        entityToIndexMap.erase(entity);
        indexToEntityMap.erase(indexOfLastElement);

        size--;
    }

    // Returns nullptr (instead of silently aliasing another entity's slot) when the
    // entity has no component of this type. operator[] on the index map used to insert
    // index 0 for a missing key, handing back entity #0's component -- a silent-corruption
    // bug. Callers that require the component should use getData and check the result.
    T* tryGetData(Entity entity) {
        auto it = entityToIndexMap.find(entity);
        return it == entityToIndexMap.end() ? nullptr : &(componentArray[it->second]);
    }

    T* getData(Entity entity) {
        T* data = tryGetData(entity);
        assert(data != nullptr && "getData: entity has no component of this type.");
        return data;
    }

    size_t count() const { return size; }

    // Iterate the dense storage directly (cache-friendly, no per-entity hash lookup),
    // invoking fn(Entity, T&) for each live component. Do not add/remove components of type
    // T from within the callback (it mutates this storage).
    template<typename Fn>
    void forEach(Fn&& fn) {
        for (size_t i = 0; i < size; ++i) {
            auto it = indexToEntityMap.find(i);
            if (it != indexToEntityMap.end()) {
                fn(it->second, componentArray[i]);
            }
        }
    }

    void entityDestroyed(Entity entity) override {
        if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
            // Remove the entity's component if it existed
            removeData(entity);
        }
    }

   private:
    // The packed array of components (of generic type T),
    // set to a specified maximum amount, matching the maximum number
    // of entities allowed to exist simultaneously, so that each entity
    // has a unique spot.
    std::vector<T> componentArray;

    // Map from an entity ID to an array index.
    std::unordered_map<Entity, size_t> entityToIndexMap;

    // Map from an array index to an entity ID.
    std::unordered_map<size_t, Entity> indexToEntityMap;

    // Total size of valid entries in the array.
    size_t size = 0;
};

class ComponentManager {
   public:
    template<typename T>
    void registerComponent() {
        auto const& type = typeid(T);

        assert(componentTypes.find(type) == componentTypes.end() && "Registering component type more than once.");
        // The signature is a bitset<64>; a component type index >= 64 would throw from
        // signature.set() at runtime.
        assert(nextComponentType < 64 && "Too many component types registered (max 64).");

        // Add this component type to the component type map
        componentTypes.insert({type, nextComponentType});

        // Create a ComponentArray pointer and add it to the component arrays map
        componentArrays.insert({type, std::make_shared<ComponentArray<T>>()});

        // Increment the value so that the next component registered will be different
        ++nextComponentType;
    }

    template<typename T>
    ComponentType getComponentType() const {
        auto const& type = typeid(T);
        // Return this component's type - used for creating signatures
        return componentTypes.at(type);
    }

    template<typename T>
    void addComponent(Entity entity, T component) {
        // Add a component to the array for an entity; move to avoid an extra copy of
        // potentially heavy components.
        getComponentArray<T>().insertData(entity, std::move(component));
    }

    template<typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>().removeData(entity);
    }

    template<typename T>
    T* getComponent(Entity entity) {
        // Get a reference to a component from the array for an entity
        return getComponentArray<T>().getData(entity);
    }

    // Returns nullptr if the type is not registered or the entity has no such component,
    // instead of asserting/throwing. For callers that legitimately probe for a component.
    template<typename T>
    T* tryGetComponent(Entity entity) {
        auto* arr = tryGetComponentArrayPtr<T>();
        return arr == nullptr ? nullptr : arr->tryGetData(entity);
    }

    // View iteration: invokes fn(Entity, Primary&, Others&...) for every entity that has all
    // of the listed component types. Iterates Primary's dense storage (cache-friendly) and
    // probes the others by hash lookup, so list the rarest component first. Behaves like a
    // minimal entt-style view. Do not add/remove any of these component types inside fn.
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
    // Map from type string pointer to a component type
    std::unordered_map<std::type_index, ComponentType> componentTypes{};

    // Map from type string pointer to a component array
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> componentArrays{};

    // The component type to be assigned to the next registered component - starting at 0
    ComponentType nextComponentType{};

    // Reference to the ComponentArray of type T. Returns a reference (not a shared_ptr
    // copy) to avoid an atomic refcount round-trip on every component access. .at() throws
    // std::out_of_range for an unregistered type instead of operator[] inserting a null.
    template<typename T>
    ComponentArray<T>& getComponentArray() {
        return static_cast<ComponentArray<T>&>(*componentArrays.at(typeid(T)));
    }

    // Non-throwing variant: nullptr if type T is not registered.
    template<typename T>
    ComponentArray<T>* tryGetComponentArrayPtr() {
        auto it = componentArrays.find(typeid(T));
        return it == componentArrays.end() ? nullptr : static_cast<ComponentArray<T>*>(it->second.get());
    }
};
}  // namespace ICE

#endif  //ICE_COMPONENT_H
