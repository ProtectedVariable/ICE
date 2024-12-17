#include "Editor.h"

#include <dialog.h>

#include <filesystem>

Editor::Editor(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_material_popup(engine),
      m_scene_popup(engine),
      m_open_scene_popup(engine) {
    m_viewport = std::make_unique<Viewport>(engine, [this]() { m_inspector->setSelectedEntity(m_hierarchy->getSelectedEntity(), true); });
    m_hierarchy = std::make_unique<Hierarchy>(engine);
    m_inspector = std::make_unique<Inspector>(engine);
    m_assets = std::make_unique<Assets>(engine, g_factory);
    ui.registerCallback("open_scene_clicked", [this] { m_open_scene_popup.open(); });
    ui.registerCallback("new_scene_clicked", [this] { m_scene_popup.open(); });
    ui.registerCallback("new_material_clicked", [this] {
        auto material = std::make_shared<ICE::Material>();
        auto path = ICE::AssetPath::WithTypePrefix<ICE::Material>("");
        auto import_name = "New Material";
        int i = 0;
        do {
            path.setName(import_name + std::to_string(i++));
        } while (m_engine->getAssetBank()->nameInUse(path));
        m_engine->getAssetBank()->addAsset(path, material);
        m_material_popup.open(m_engine->getAssetBank()->getUID(path));
    });
    ui.registerCallback("import_mesh_clicked", [this] {
        std::filesystem::path file = open_native_dialog("*.obj");
        if (!file.empty()) {
            std::string import_name = "Imported Mesh ";
            int i = 0;
            do {
                import_name + std::to_string(++i);
            } while (m_engine->getAssetBank()->nameInUse(ICE::AssetPath::WithTypePrefix<ICE::Mesh>(import_name + std::to_string(i))));
            import_name = import_name + std::to_string(i);
            m_engine->getProject()->copyAssetFile("Meshes", import_name, file);
            m_engine->getAssetBank()->addAsset<ICE::Mesh>(
                import_name, {m_engine->getProject()->getBaseDirectory() / "Assets" / "Meshes" / (import_name + file.extension().string())});
            m_assets->rebuildViewer();
        }
    });
    ui.registerCallback("import_tex2d_clicked", [this] {
        std::filesystem::path file = open_native_dialog("*.png");
        if (!file.empty()) {
            std::string import_name = "Imported Texture ";
            int i = 0;
            do {
                import_name + std::to_string(++i);
            } while (m_engine->getAssetBank()->nameInUse(ICE::AssetPath::WithTypePrefix<ICE::Texture2D>(import_name + std::to_string(i))));
            import_name = import_name + std::to_string(i);
            m_engine->getProject()->copyAssetFile("Textures", import_name, file);
            m_engine->getAssetBank()->addAsset<ICE::Texture2D>(
                import_name, {m_engine->getProject()->getBaseDirectory() / "Assets" / "Textures" / (import_name + file.extension().string())});
            m_assets->rebuildViewer();
        }
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
    m_viewport->setSelectedEntity(m_selected_entity);

    m_material_popup.render();
    m_scene_popup.render();
    m_open_scene_popup.render();

    if (m_material_popup.accepted()) {
        m_assets->rebuildViewer();
    }
    if (m_scene_popup.accepted()) {
        m_engine->getProject()->addScene(ICE::Scene(m_scene_popup.getSceneName()));
        m_engine->getProject()->setCurrentScene(m_engine->getProject()->getScenes().back());
        m_engine->setupScene();
        m_hierarchy->setSelectedEntity(0);
        m_viewport->setSelectedEntity(0);
        m_inspector->setSelectedEntity(0);
    }
    if (m_open_scene_popup.accepted()) {
        m_engine->getProject()->setCurrentScene(m_engine->getProject()->getScenes()[m_open_scene_popup.getSelectedIndex()]);
        m_engine->setupScene();
        m_hierarchy->setSelectedEntity(0);
        m_viewport->setSelectedEntity(0);
        m_inspector->setSelectedEntity(0);
    }

    return m_done;
}