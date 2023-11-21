#include "Inspector.h"

Inspector::Inspector(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
}

bool Inspector::update() {
    ui.render();
    return m_done;
}

void Inspector::setSelectedEntity(ICE::Entity e) {
    if (m_selected_entity == e) {
        return;
    }
    m_selected_entity = e;
    auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
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
