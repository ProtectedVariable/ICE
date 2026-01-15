#include "Assets.h"

#include <ForwardRenderer.h>
#include <PerspectiveCamera.h>

Assets::Assets(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_g_factory(g_factory),
      m_renderer(engine->getApi(), g_factory, engine->getGPURegistry()),
      ui(m_asset_categories,
         m_engine->getGPURegistry()->getTexture2D(ICE::AssetPath::WithTypePrefix<ICE::Texture2D>("Editor/folder"))->ptr()),
      m_material_editor(engine) {
    rebuildViewer();
    ui.registerCallback("material_duplicate", [this](std::string name) {
        auto id = m_engine->getAssetBank()->getUID("Materials/" + name);
        auto mat_copy = std::make_shared<ICE::Material>(*m_engine->getAssetBank()->getAsset<ICE::Material>(id));
        mat_copy->setSources({});
        m_engine->getAssetBank()->addAsset<ICE::Material>(name + " copy", mat_copy);
        rebuildViewer();
    });
    ui.registerCallback("delete_asset", [this](std::string name) {
        m_engine->getAssetBank()->removeAsset(name);
        rebuildViewer();
    });

    ui.registerCallback("asset_category_selected", [this](int index) {
        m_current_category_index = index;
        ui.setCurrentView(m_asset_views[m_current_category_index]);
    });

    ui.registerCallback("item_clicked", [this](std::string label) {
        auto current_view = ui.getCurrentView();
        if (label == "..") {
            ui.setCurrentView(*current_view.parent);
        } else {
            for (const auto& folder : current_view.subfolders) {
                if (folder.folder_name == label) {
                    ui.setCurrentView(folder);
                    return;
                }
            }
            for (const auto& asset : current_view.assets) {
                if (asset.name == label) {
                    handleClick(asset);
                    return;
                }
            }
        }
    });
    ui.registerCallback("item_selected", [this](std::string label) {
        auto current_view = ui.getCurrentView();
        for (const auto& asset : current_view.assets) {
            if (asset.name == label) {
                m_current_preview.emplace(asset);
                return;
            }
        }
    });

}

void Assets::rebuildViewer() {
    m_asset_views.clear();
    for (const auto& category : m_asset_categories) {
        AssetView folder_view;
        folder_view.folder_name = category;
        m_asset_views.push_back(folder_view);
    }

    auto asset_bank = m_engine->getAssetBank();
    for (const auto& entry : asset_bank->getAllEntries()) {
        Thumbnail thumbnail;
        auto [ptr, flip] = m_renderer.createThumbnail(entry.asset, entry.path.toString());
        thumbnail.ptr = ptr;
        thumbnail.flip = flip;
        std::string category;
        if (std::dynamic_pointer_cast<ICE::Model>(entry.asset)) {
            category = "Models";
        } else if (std::dynamic_pointer_cast<ICE::Mesh>(entry.asset)) {
            category = "Meshes";
        } else if (std::dynamic_pointer_cast<ICE::Material>(entry.asset)) {
            category = "Materials";
        } else if (std::dynamic_pointer_cast<ICE::Texture2D>(entry.asset)) {
            category = "Textures2D";
        } else if (std::dynamic_pointer_cast<ICE::TextureCube>(entry.asset)) {
            category = "TextureCubes";
        } else if (std::dynamic_pointer_cast<ICE::Shader>(entry.asset)) {
            category = "Shaders";
        } else {
            category = "Others";
        }
        for (auto& folder_view : m_asset_views) {
            if (folder_view.folder_name == category) {
                auto paths = entry.path.getPath();
                paths.push_back(entry.path.getName());
                createSubfolderView(&folder_view, std::vector<std::string>(paths.begin() + 1, paths.end()), thumbnail, entry.path.toString());
                break;
            }
        }
    }
    ui.setCurrentView(m_asset_views[m_current_category_index]);
}

void Assets::handleClick(const AssetData& data) {
    auto asset_ptr = m_engine->getAssetBank()->getAsset(m_engine->getAssetBank()->getUID(m_current_preview.value().asset_path));
    if (auto m = std::dynamic_pointer_cast<ICE::Texture2D>(asset_ptr); m) {
        return;  //TODO: Maybe popup window with the texture
    } else if (auto m = std::dynamic_pointer_cast<ICE::TextureCube>(asset_ptr); m) {
        return;  //TODO: Maybe popup window with the texture
    } else if (auto m = std::dynamic_pointer_cast<ICE::Shader>(asset_ptr); m) {
        return;  //TODO: Shader editor
    } else if (auto m = std::dynamic_pointer_cast<ICE::Model>(asset_ptr); m) {
        return;  // Probably no action
    } else if (auto m = std::dynamic_pointer_cast<ICE::Material>(asset_ptr); m) {
        m_material_editor.open(data.asset_path);
    }
}

void Assets::createSubfolderView(AssetView* parent_view, const std::vector<std::string>& path, const Thumbnail& thumbnail,
                                 const std::string& full_path) {
    if (path.size() == 1) {
        parent_view->assets.push_back({path.back(), thumbnail, full_path});
        return;
    }
    for (auto& folder : parent_view->subfolders) {
        if (folder.folder_name == path[0]) {
            createSubfolderView(&folder, std::vector<std::string>(path.begin() + 1, path.end()), thumbnail, full_path);
            return;
        }
    }
    parent_view->subfolders.push_back(AssetView{.parent = parent_view, .folder_name = path[0], .assets = {}, .subfolders = {}});
    createSubfolderView(&(parent_view->subfolders.back()), std::vector<std::string>(path.begin() + 1, path.end()), thumbnail, full_path);
}

bool Assets::update() {
    m_t += 0.1f;

    if (m_current_preview.has_value()) {
        auto asset_ptr = m_engine->getAssetBank()->getAsset(m_engine->getAssetBank()->getUID(m_current_preview.value().asset_path));
        ui.setPreviewTexture(m_renderer.getPreview(asset_ptr, m_current_preview.value().asset_path, m_t).first);
    }
    ui.render();
    if (m_material_editor.update()) {
        rebuildViewer();
    }
    return m_done;
}
