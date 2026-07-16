#pragma once

#include <ICEEngine.h>
#include <Model.h>
#include <UI/EditorWidget.h>
#include <UI/MaterialEditDialog.h>
#include <UI/NewSceneDialog.h>
#include <UI/OpenSceneDialog.h>
#include <dialog.h>

#include <memory>
#include <type_traits>
#include <vector>

#include "Assets.h"
#include "Controller.h"
#include "Hierarchy.h"
#include "Inspector.h"
#include "Viewport.h"

class Editor : public Controller {
   public:
    Editor(const std::shared_ptr<ICE::ICEEngine> &engine, const std::shared_ptr<ICE::GraphicsFactory> &g_factory);
    bool update() override;

   private:
    template<typename T>
    bool importAsset(const std::vector<FileFilter> &filters = {}) {
        std::filesystem::path file = open_native_dialog(filters);
        if (file.empty()) {
            return false;
        }
        std::string import_name = file.stem().string();
        int i = 0;
        while (m_engine->getAssetBank()->nameInUse(ICE::AssetPath::WithTypePrefix<T>(import_name))) {
            import_name = file.stem().string() + std::to_string(++i);
        }
        auto folder = ICE::AssetPath::WithTypePrefix<T>("").getPath().at(0);
        m_engine->getProject()->copyAssetFile(folder, import_name, file);
        std::vector<std::filesystem::path> sources = {
            m_engine->getProject()->getBaseDirectory() / "Assets" / folder / (import_name + file.extension().string())};

        // Async import: reserve the UID now and load in the background (staging off-thread once
        // enableBackgroundAssetLoading is on). AssetBank::pump() -- driven by ICEEngine::step --
        // publishes the asset, and the viewer rebuilds when the in-flight count drops (see
        // Assets::update). Model import routes through the two-phase stage/commit path because its
        // loader mutates the bank; other kinds use the pure-loader convenience overload.
        if constexpr (std::is_same_v<T, ICE::Model>) {
            m_engine->getProject()->requestModel(import_name, sources);
        } else {
            m_engine->getAssetBank()->requestAsset<T>(import_name, sources);
        }
        return true;
    }

    void loadScene(int index);

    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    EditorWidget ui;
    std::unique_ptr<Viewport> m_viewport;
    std::unique_ptr<Hierarchy> m_hierarchy;
    std::unique_ptr<Inspector> m_inspector;
    std::unique_ptr<Assets> m_assets;
    ICE::Entity m_selected_entity = 0;
    bool m_entity_transform_changed = false;

    //Popups
    MaterialEditor m_material_popup;
    ShaderEditor m_shader_popup;
    NewSceneDialog m_scene_popup;
    OpenSceneDialog m_open_scene_popup;
};
