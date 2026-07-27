#include "Editor.h"

#include <filesystem>

Editor::Editor(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_scene_popup(engine),
      m_open_scene_popup(engine),
      m_material_popup(engine),
      m_shader_popup(engine) {
    // Stage imports off the main thread so importing a large model doesn't hitch the editor. The
    // asset bank publishes results on the main thread via ICEEngine::step -> pump().
    m_engine->enableBackgroundAssetLoading();
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
    ui.registerCallback("new_shader_menu", [this] { m_shader_popup.open(ICE::AssetPath("")); });
    ui.registerCallback("new_material_menu", [this] { m_material_popup.open(ICE::AssetPath("")); });
    ui.registerCallback("import_material_menu", [this] { importAsset<ICE::Material>({{"ICE Material", "*.icm"}}); });
    ui.registerCallback("import_texture2d_menu", [this] { importAsset<ICE::Texture2D>({{"Images", "*.png;*.jpg;*.jpeg"}}); });
    ui.registerCallback("import_cubemap_menu", [this] { importAsset<ICE::TextureCube>({{"Images", "*.png;*.jpg;*.jpeg"}}); });
    ui.registerCallback("import_model_menu", [this] { importAsset<ICE::Model>({{"Models", "*.glb;*.fbx;*.obj"}}); });
    ui.registerCallback("import_audio_menu", [this] { importAudioAsset(false); });
    ui.registerCallback("import_audio_3d_menu", [this] { importAudioAsset(true); });
    ui.registerCallback("audio_mixer_menu", [this] { m_audio_mixer.open(); });
    ui.registerCallback("save_menu", [this] {
        // Harvest the live mixer levels onto the project first, so a mix tweaked in the Audio
        // Mixer panel is part of what gets written.
        m_engine->storeProjectMixer();
        m_engine->getProject()->writeToFile(m_engine->getCamera());
    });
    ui.registerCallback("exit_menu", [this] {
        m_engine->getProject()->writeToFile(m_engine->getCamera());
        m_engine->getWindow()->close();
    });
}

bool Editor::update() {
    ui.render();
    // The audio service is built lazily on first use, so bind it every frame rather than once at
    // construction (where there may be no project yet).
    if (auto* audio = m_engine->audio()) {
        if (!m_audio_initialized) {
            // This editor has no play mode: a loaded scene is live and its `play on awake` sources
            // would start the moment a project opens. Start muted so opening a project is quiet;
            // the Audio > Mixer panel un-mutes. Revisit if a play/stop mode is ever added.
            audio->setMuted(true);
            m_audio_initialized = true;
        }
        m_audio_mixer.setAudioEngine(audio);
    }
    m_audio_mixer.render();
    m_viewport->update();
    m_hierarchy->update();
    m_inspector->update();
    m_assets->update();

    m_selected_entity = m_hierarchy->getSelectedEntity();

    m_inspector->setSelectedEntity(m_selected_entity);
    // A hierarchy rename keeps the same entity selected, so force the Inspector to reload its
    // (otherwise cached) name field.
    if (m_hierarchy->selectionRenamed()) {
        m_inspector->setSelectedEntity(m_selected_entity, true);
    }
    if (m_inspector->entityHasChanged()) {
        m_hierarchy->rebuildTree();
    }
    m_viewport->setSelectedEntity(m_selected_entity);

    if (m_material_popup.update()) {
        m_assets->rebuildViewer();
    }

    if (m_shader_popup.update()) {
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