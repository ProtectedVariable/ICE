#pragma once

#include <Entity.h>
#include <Registry.h>
#include <TransformComponent.h>
#include <RenderComponent.h>
#include <LightComponent.h>
#include <AnimationComponent.h>

#include <memory>

namespace ICE {

/**
 * @brief Helper class to simplify entity component access
 * 
 * Reduces verbosity when working with entities by providing
 * direct access to components without repeated registry calls.
 * 
 * Example:
 *   EntityHelper entity(e, registry);
 *   entity.transform()->setPosition({0, 1, 0});
 *   // vs
 *   registry->getComponent<TransformComponent>(e)->setPosition({0, 1, 0});
 */
class EntityHelper {
public:
    EntityHelper(Entity id, std::shared_ptr<Registry> registry)
        : m_id(id), m_registry(registry) {}

    EntityHelper(Entity id, Registry* registry)
        : m_id(id), m_registry_ptr(registry) {}

    // Component access shortcuts
    TransformComponent* transform() {
        return getRegistry()->getComponent<TransformComponent>(m_id);
    }

    RenderComponent* render() {
        return getRegistry()->getComponent<RenderComponent>(m_id);
    }

    LightComponent* light() {
        return getRegistry()->getComponent<LightComponent>(m_id);
    }

    AnimationComponent* animation() {
        return getRegistry()->getComponent<AnimationComponent>(m_id);
    }

    // Generic component access
    template<typename T>
    T* getComponent() {
        return getRegistry()->getComponent<T>(m_id);
    }

    template<typename T>
    void addComponent(T component) {
        getRegistry()->addComponent<T>(m_id, component);
    }

    template<typename T>
    void removeComponent() {
        getRegistry()->removeComponent<T>(m_id);
    }

    template<typename T>
    bool hasComponent() {
        return getRegistry()->entityHasComponent<T>(m_id);
    }

    // Entity ID access
    Entity id() const { return m_id; }

    // Validity check
    bool isValid() const { return m_id != 0 && getRegistry() != nullptr; }

private:
    Registry* getRegistry() const {
        if (auto reg = m_registry.lock()) {
            return reg.get();
        }
        return nullptr;
    }

    Entity m_id;
    std::weak_ptr<Registry> m_registry;
};

}  // namespace ICE
