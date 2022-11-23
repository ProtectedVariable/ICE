//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_COMPONENT_H
#define ICE_COMPONENT_H

#include <Scene/Entity.h>

namespace ICE {

    using ComponentType = std::uint8_t;

    struct Component {
    };


    class IComponentArray
    {
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
            componentArray[newIndex] = component;
            size++;
        }

        void removeData(Entity entity)
        {
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

        T* getData(Entity entity)
        {
            assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving non-existent component.");

            // Return a reference to the entity's component
            return &(componentArray[entityToIndexMap[entity]]);
        }

        void entityDestroyed(Entity entity) override
        {
            if (entityToIndexMap.find(entity) != entityToIndexMap.end())
            {
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
        size_t size;
    };

    class ComponentManager {
    public:
        template<typename T>
        void registerComponent()
        {
            const char* typeName = typeid(T).name();

            assert(componentTypes.find(typeName) == componentTypes.end() && "Registering component type more than once.");

            // Add this component type to the component type map
            componentTypes.insert({typeName, nextComponentType});

            // Create a ComponentArray pointer and add it to the component arrays map
            componentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

            // Increment the value so that the next component registered will be different
            ++nextComponentType;
        }

        template<typename T>
        ComponentType getComponentType()
        {
            const char* typeName = typeid(T).name();

            assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");

            // Return this component's type - used for creating signatures
            return componentTypes[typeName];
        }

        template<typename T>
        void addComponent(Entity entity, T component)
        {
            // Add a component to the array for an entity
            getComponentArray<T>()->insertData(entity, component);
        }

        template<typename T>
        void removeComponent(Entity entity)
        {
            // Remove a component from the array for an entity
            getComponentArray<T>()->removeData(entity);
        }

        template<typename T>
        T* getComponent(Entity entity)
        {
            // Get a reference to a component from the array for an entity
            return getComponentArray<T>()->getData(entity);
        }

        void entityDestroyed(Entity entity)
        {
            // Notify each component array that an entity has been destroyed
            // If it has a component for that entity, it will remove it
            for (auto const& pair : componentArrays)
            {
                auto const& component = pair.second;

                component->entityDestroyed(entity);
            }
        }

    private:
        // Map from type string pointer to a component type
        std::unordered_map<const char*, ComponentType> componentTypes{};

        // Map from type string pointer to a component array
        std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays{};

        // The component type to be assigned to the next registered component - starting at 0
        ComponentType nextComponentType{};

        // Convenience function to get the statically casted pointer to the ComponentArray of type T.
        template<typename T>
        std::shared_ptr<ComponentArray<T>> getComponentArray()
        {
            const char* typeName = typeid(T).name();

            assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");

            return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
        }
    };
}

#endif //ICE_COMPONENT_H
