#include "Inspector.h"

Inspector::Inspector(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    ui.registerCallback("entity_name_changed", [this](const std::string& text) {
        m_engine->getProject()->getCurrentScene()->setAlias(m_selected_entity, text);
        m_entity_has_changed++;
    });
}

bool Inspector::update() {
    ui.render();
    return m_done;
}

bool Inspector::entityHasChanged() {
    if (m_entity_has_changed > 0) {
        m_entity_has_changed--;
    }
    return (m_entity_has_changed + 1) != 0;
}

void Inspector::setSelectedEntity(ICE::Entity e) {
    if (m_selected_entity == e) {
        return;
    }
    m_entity_has_changed = 0;
    m_selected_entity = e;
    auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();

    ui.setEntityName(m_engine->getProject()->getCurrentScene()->getAlias(e));

    if (registry->entityHasComponent<ICE::TransformComponent>(e)) {
        auto tc = registry->getComponent<ICE::TransformComponent>(e);
        ui.setTransformComponent(tc);
    }
    if (registry->entityHasComponent<ICE::RenderComponent>(e)) {
        auto rc = registry->getComponent<ICE::RenderComponent>(e);

        auto meshes = m_engine->getAssetBank()->getAll<ICE::Mesh>();
        auto materials = m_engine->getAssetBank()->getAll<ICE::Material>();

        std::vector<std::string> meshes_paths, materials_paths;
        std::vector<ICE::AssetUID> meshes_ids, materials_ids;

        for (const auto& [id, m] : meshes) {
            meshes_ids.push_back(id);
            meshes_paths.push_back(m_engine->getAssetBank()->getName(id).toString());
        }

        for (const auto& [id, m] : materials) {
            materials_ids.push_back(id);
            materials_paths.push_back(m_engine->getAssetBank()->getName(id).toString());
        }

        ui.setRenderComponent(rc, meshes_paths, meshes_ids, materials_paths, materials_ids);
    }
}
