#include "Inspector.h"
#include <Model.h>
#include <SkeletonPoseComponent.h>

Inspector::Inspector(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    ui.registerCallback("entity_name_changed", [this](std::string text) {
        m_engine->getProject()->getCurrentScene()->setAlias(m_selected_entity, text);
        m_entity_has_changed++;
    });
    ui.registerCallback("add_component_clicked", [this] {
        m_add_component_popup.setData(m_engine->getProject()->getCurrentScene()->getRegistry(), m_selected_entity);
        m_add_component_popup.open();
    });
    // Defer the actual removal to after render() (see PendingRemove): these fire from inside
    // the component widget's own render pass.
    ui.registerCallback("remove_light_component_clicked", [this] { m_pending_remove = PendingRemove::Light; });
    ui.registerCallback("remove_render_component_clicked", [this] { m_pending_remove = PendingRemove::Render; });
    ui.registerCallback("remove_animation_component_clicked", [this] { m_pending_remove = PendingRemove::Animation; });
}

bool Inspector::update() {
    // Refresh the widgets' cached component pointers from the registry every frame. A
    // component vector can be reallocated by another entity's add/remove between frames,
    // which would leave the Inspector writing through a dangling pointer. tryGetComponent
    // returns nullptr for a missing/deleted entity, so this also clears the widgets safely.
    if (m_engine->getProject() && m_engine->getProject()->getCurrentScene()) {
        auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
        ICE::Entity e = m_selected_entity;
        auto tc = registry->tryGetComponent<ICE::TransformComponent>(e);
        auto lc = registry->tryGetComponent<ICE::LightComponent>(e);
        auto rc = registry->tryGetComponent<ICE::RenderComponent>(e);
        ICE::AnimationComponent* ac = nullptr;
        if (registry->tryGetComponent<ICE::SkeletonPoseComponent>(e) != nullptr) {
            ac = registry->tryGetComponent<ICE::AnimationComponent>(e);
        }
        ui.refreshComponents(tc, lc, rc, ac);
    }

    ui.render();

    // Apply a deferred component removal now that the widgets have finished rendering.
    if (m_pending_remove != PendingRemove::None) {
        auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
        if (m_pending_remove == PendingRemove::Render) {
            registry->removeComponent<ICE::RenderComponent>(m_selected_entity);
        } else if (m_pending_remove == PendingRemove::Light) {
            registry->removeComponent<ICE::LightComponent>(m_selected_entity);
        } else if (m_pending_remove == PendingRemove::Animation) {
            // Remove only the AnimationComponent: the skeleton pose and skinning stay intact so
            // the mesh keeps rendering (frozen at its last pose) instead of losing its bone data.
            registry->removeComponent<ICE::AnimationComponent>(m_selected_entity);
        }
        m_pending_remove = PendingRemove::None;
        setSelectedEntity(m_selected_entity, true);
    }

    if (m_add_component_popup.isOpen()) {
        m_add_component_popup.render();
        if (m_add_component_popup.getResult() == DialogResult::Ok) {
            setSelectedEntity(m_selected_entity, true);
        }
    }

    return m_done;
}

bool Inspector::entityHasChanged() {
    // Return true only when there is a pending change to consume. The old expression
    // (value + 1) != 0 was always true, so the hierarchy tree was rebuilt every frame.
    if (m_entity_has_changed > 0) {
        m_entity_has_changed--;
        return true;
    }
    return false;
}

void Inspector::setSelectedEntity(ICE::Entity e, bool force_refesh) {
    if (!m_engine->getProject()->getCurrentScene()->hasEntity(e)) {
        return;
    }
    if (m_selected_entity == e && !force_refesh) {
        return;
    }
    m_entity_has_changed = 0;
    m_selected_entity = e;

    auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
    ui.setEntityName(m_engine->getProject()->getCurrentScene()->getAlias(e));
    ui.setTransformComponent(nullptr);
    ui.setLightComponent(nullptr);
    ui.setAnimationComponent(nullptr, {});
    ui.setRenderComponent(nullptr, {}, {}, {}, {});

    if (registry->entityHasComponent<ICE::TransformComponent>(e)) {
        auto tc = registry->getComponent<ICE::TransformComponent>(e);
        ui.setTransformComponent(tc);
    }
    if (registry->entityHasComponent<ICE::RenderComponent>(e)) {
        auto rc = registry->getComponent<ICE::RenderComponent>(e);

        auto meshes = m_engine->getAssetBank()->getAll<ICE::Mesh>();

        std::vector<std::string> meshes_paths;
        std::vector<ICE::AssetUID> meshes_ids;

        for (const auto& [id, m] : meshes) {
            meshes_ids.push_back(id);
            meshes_paths.push_back(m_engine->getAssetBank()->getName(id).toString());
        }

        auto materials = m_engine->getAssetBank()->getAll<ICE::Material>();

        std::vector<std::string> materials_paths;
        std::vector<ICE::AssetUID> materials_ids;

        for (const auto& [id, m] : materials) {
            materials_ids.push_back(id);
            materials_paths.push_back(m_engine->getAssetBank()->getName(id).toString());
        }

        ui.setRenderComponent(rc, meshes_paths, meshes_ids, materials_paths, materials_ids);
    }
    if (registry->entityHasComponent<ICE::AnimationComponent>(e) && registry->entityHasComponent<ICE::SkeletonPoseComponent>(e)) {
        auto ac = registry->getComponent<ICE::AnimationComponent>(e);
        auto spc = registry->getComponent<ICE::SkeletonPoseComponent>(e);
        auto anims = m_engine->getAssetBank()->getAsset<ICE::Model>(spc->skeletonModel)->getAnimations();
        ui.setAnimationComponent(ac, anims);
    }
    if (registry->entityHasComponent<ICE::LightComponent>(e)) {
        auto lc = registry->getComponent<ICE::LightComponent>(e);
        ui.setLightComponent(lc);
    }
}
