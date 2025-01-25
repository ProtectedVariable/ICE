#include "Inspector.h"

Inspector::Inspector(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    ui.registerCallback("entity_name_changed", [this](std::string text) {
        m_engine->getProject()->getCurrentScene()->setAlias(m_selected_entity, text);
        m_entity_has_changed++;
    });
    ui.registerCallback("add_component_clicked",
                        [this] { m_add_component_popup.open(m_engine->getProject()->getCurrentScene()->getRegistry(), m_selected_entity); });
    ui.registerCallback("remove_light_component_clicked", [this] {
        m_engine->getProject()->getCurrentScene()->getRegistry()->removeComponent<ICE::LightComponent>(m_selected_entity);
        setSelectedEntity(m_selected_entity, true);
    });
    ui.registerCallback("remove_render_component_clicked", [this] {
        m_engine->getProject()->getCurrentScene()->getRegistry()->removeComponent<ICE::RenderComponent>(m_selected_entity);
        setSelectedEntity(m_selected_entity, true);
    });
}

bool Inspector::update() {
    m_add_component_popup.render();
    ui.render();
    if (m_add_component_popup.accepted()) {
        setSelectedEntity(m_selected_entity, true);
    }
    return m_done;
}

bool Inspector::entityHasChanged() {
    if (m_entity_has_changed > 0) {
        m_entity_has_changed--;
    }
    return (m_entity_has_changed + 1) != 0;
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
    ui.setLightComponent(nullptr);
    ui.setRenderComponent(nullptr, {}, {});

    if (registry->entityHasComponent<ICE::TransformComponent>(e)) {
        auto tc = registry->getComponent<ICE::TransformComponent>(e);
        ui.setTransformComponent(tc);
    }
    if (registry->entityHasComponent<ICE::RenderComponent>(e)) {
        auto rc = registry->getComponent<ICE::RenderComponent>(e);

        auto meshes = m_engine->getAssetBank()->getAll<ICE::Model>();

        std::vector<std::string> meshes_paths;
        std::vector<ICE::AssetUID> meshes_ids;

        for (const auto& [id, m] : meshes) {
            meshes_ids.push_back(id);
            meshes_paths.push_back(m_engine->getAssetBank()->getName(id).toString());
        }

        ui.setRenderComponent(rc, meshes_paths, meshes_ids);
    }
    if (registry->entityHasComponent<ICE::LightComponent>(e)) {
        auto lc = registry->getComponent<ICE::LightComponent>(e);
        ui.setLightComponent(lc);
    }
}
