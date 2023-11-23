#include "Editor.h"

Editor::Editor(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_material_popup(engine) {
    m_viewport = std::make_unique<Viewport>(engine);
    m_hierarchy = std::make_unique<Hierarchy>(engine);
    m_inspector = std::make_unique<Inspector>(engine);
    m_assets = std::make_unique<Assets>(engine, g_factory);
    ui.registerCallback("new_material_clicked", [this] {
        auto material = std::make_shared<ICE::Material>();
        auto path = ICE::AssetPath::WithTypePrefix<ICE::Material>("New Material");
        m_engine->getAssetBank()->addAsset(path, material);
        m_material_popup.open(m_engine->getAssetBank()->getUID(path));
    });
}

bool Editor::update() {
    ui.render();
    m_viewport->update();
    m_hierarchy->update();
    m_inspector->update();
    m_assets->update();

    m_selected_entity = m_hierarchy->getSelectedEntity();

    m_inspector->setSelectedEntity(m_selected_entity);
    if (m_inspector->entityHasChanged()) {
        m_hierarchy->rebuildTree();
    }

    m_material_popup.render();

    if (m_material_popup.accepted()) {
        m_assets->rebuildViewer();
    }

    return m_done;
}