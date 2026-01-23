#include "Editor.h"

#include <filesystem>

Editor::Editor(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_scene_popup(engine),
      m_open_scene_popup(engine),
      m_material_popup(engine) {
    m_viewport = std::make_unique<Viewport>(
        engine, [this]() { m_inspector->setSelectedEntity(m_hierarchy->getSelectedEntity(), true); },
        [this](ICE::Entity e) {
            if (m_engine->getProject()->getCurrentScene()->hasEntity(e))
                m_hierarchy->setSelectedEntity(e);
        });
    m_hierarchy = std::make_unique<Hierarchy>(engine);
    m_inspector = std::make_unique<Inspector>(engine);
    m_assets = std::make_unique<Assets>(engine, g_factory);
    ui.registerCallback("open_scene_menu", [this] { m_open_scene_popup.open(); });
    ui.registerCallback("new_scene_menu", [this] { m_scene_popup.open(); });
    ui.registerCallback("new_shader_menu", [this] {});
    ui.registerCallback("new_material_menu", [this] { m_material_popup.open(ICE::AssetPath("")); });
    ui.registerCallback("import_material_menu", [this] { importAsset<ICE::Material>({{"ICE Material", "*.icm"}}); });
    ui.registerCallback("import_texture2d_menu", [this] { importAsset<ICE::Texture2D>({{"Images", "*.png;*.jpg;*.jpeg"}}); });
    ui.registerCallback("import_cubemap_menu", [this] { importAsset<ICE::TextureCube>({{"Images", "*.png;*.jpg;*.jpeg"}}); });
    ui.registerCallback("import_shader_menu", [this] {});
    ui.registerCallback("import_model_menu", [this] { importAsset<ICE::Model>({{"Models", "*.glb;*.fbx;*.obj"}}); });
    ui.registerCallback("save_menu", [this] { m_engine->getProject()->writeToFile(m_engine->getCamera()); });
    ui.registerCallback("exit_menu", [this] {
        m_engine->getProject()->writeToFile(m_engine->getCamera());
        m_engine->getWindow()->close();
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

    if (m_material_popup.update()) {
        m_assets->rebuildViewer();
    }

    if (m_scene_popup.isOpen()) {
        m_scene_popup.render();
        if (m_scene_popup.getResult() == DialogResult::Ok) {
            m_engine->getProject()->addScene(ICE::Scene(m_scene_popup.getSceneName()));
            loadScene(m_engine->getProject()->getScenes().size() - 1);
        }
    }

    if (m_open_scene_popup.isOpen()) {
        m_open_scene_popup.render();
        if (m_open_scene_popup.getResult() == DialogResult::Ok) {
            loadScene(m_open_scene_popup.getSelectedIndex());
        }
    }
    return m_done;
}

void Editor::loadScene(int index) {
    m_engine->getProject()->setCurrentScene(m_engine->getProject()->getScenes().at(index));
    m_engine->setupScene(m_engine->getCamera());
    m_hierarchy->setSelectedEntity(0);
    m_viewport->setSelectedEntity(0);
    m_inspector->setSelectedEntity(0);
}