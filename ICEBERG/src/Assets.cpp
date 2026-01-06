#include "Assets.h"

#include <ForwardRenderer.h>
#include <PerspectiveCamera.h>

Assets::Assets(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_g_factory(g_factory),
      ui(m_asset_categories, ICE::ForwardRenderer(engine->getApi(), g_factory)) {
    rebuildViewer();
    /* ui.registerCallback("material_edit",
                             [this](std::string name) { m_material_widget.open(m_engine->getAssetBank()->getUID("Materials/" + name)); });
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
    });*/

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
                if (asset.first == label) {
                    //TODO: Handle
                    return;
                }
            }
        }
    });
    ui.registerCallback("item_clicked", [this](std::string label) {
        auto current_view = ui.getCurrentView();
        for (const auto& asset : current_view.assets) {
            if (asset.first == label) {

                return;
            }
        }
    });

    ui.setCurrentView(m_asset_views[m_current_category_index]);
}

void* Assets::createThumbnail(const ICE::AssetBankEntry& entry) {
    auto& asset = entry.asset;
    if (auto m = std::dynamic_pointer_cast<ICE::Texture2D>(asset); m) {
        return m->getTexture();
    }

    auto model_uid = m_engine->getAssetBank()->getUID(ICE::AssetPath::WithTypePrefix<ICE::Model>("sphere"));
    auto material_uid = m_engine->getAssetBank()->getUID(ICE::AssetPath("Materials/base_mat"));

    auto self_uid = m_engine->getAssetBank()->getUID(entry.path);
    if (auto m = std::dynamic_pointer_cast<ICE::Model>(asset); m) {
        model_uid = self_uid;
    } else if (auto m = std::dynamic_pointer_cast<ICE::Material>(asset); m) {
        material_uid = self_uid;
    } else {
        //TODO return default icons
        return nullptr;
    }
    auto model = m_engine->getAssetBank()->getAsset<ICE::Model>(model_uid);
    auto material = m_engine->getAssetBank()->getAsset<ICE::Material>(material_uid);
    auto shader = m_engine->getAssetBank()->getAsset<ICE::Shader>(material->getShader());

    auto camera = std::make_shared<ICE::PerspectiveCamera>(60.0, 1.0, 0.01, 10000.0);
    camera->backward(2);
    camera->up(1);
    camera->pitch(-30);

    ICE::ForwardRenderer renderer(m_engine->getApi(), m_engine->getGraphicsFactory());
    renderer.resize(256, 256);
    std::vector<std::shared_ptr<ICE::Mesh>> meshes;
    std::vector<ICE::AssetUID> materials;
    std::vector<Eigen::Matrix4f> transforms;

    model->traverse(meshes, materials, transforms, Eigen::Matrix4f::Identity());
    std::unordered_map<ICE::AssetUID, std::shared_ptr<ICE::Texture>> textures;
    for (const auto& [k, v] : material->getAllUniforms()) {
        if (std::holds_alternative<ICE::AssetUID>(v)) {
            auto id = std::get<ICE::AssetUID>(v);
            textures.try_emplace(id, m_engine->getAssetBank()->getAsset<ICE::Texture2D>(id));
        }
    }
    for (int i = 0; i < meshes.size(); i++) {
        renderer.submitDrawable(
            ICE::Drawable{.mesh = meshes[i], .material = material, .shader = shader, .textures = textures, .model_matrix = transforms[i]});
    }
    renderer.submitLight(
        ICE::Light{.position = {-2, 2, 2}, .rotation = {0, 0, 0}, .color = {1, 1, 1}, .distance_dropoff = 0, .type = ICE::LightType::PointLight});

    renderer.prepareFrame(*camera);
    auto fb = renderer.render();
    renderer.endFrame();

    return static_cast<char*>(0) + fb->getTexture();
}

void Assets::rebuildViewer() {
    for (const auto& category : m_asset_categories) {
        AssetView folder_view;
        folder_view.folder_name = category;
        m_asset_views.push_back(folder_view);
    }

    auto asset_bank = m_engine->getAssetBank();
    for (const auto& entry : asset_bank->getAllEntries()) {
        void* thumbnail = createThumbnail(entry);
        std::string category;
        if (std::dynamic_pointer_cast<ICE::Model>(entry.asset)) {
            category = "Models";
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
                createSubfolderView(&folder_view, std::vector<std::string>(paths.begin() + 1, paths.end()), thumbnail);
                break;
            }
        }
    }
}

void Assets::createSubfolderView(AssetView* parent_view, const std::vector<std::string>& path, void* thumbnail) {
    if (path.size() == 1) {
        parent_view->assets.push_back({path.back(), thumbnail});
        return;
    }
    for (auto& folder : parent_view->subfolders) {
        if (folder.folder_name == path[0]) {
            createSubfolderView(&folder, std::vector<std::string>(path.begin() + 1, path.end()), thumbnail);
        }
        return;
    }
    parent_view->subfolders.push_back(AssetView{.parent = parent_view, .folder_name = path[0], .assets = {}, .subfolders = {}});
    createSubfolderView(&(parent_view->subfolders.back()), std::vector<std::string>(path.begin() + 1, path.end()), thumbnail);
}

bool Assets::update() {
    ui.render();
    //m_material_widget.render();
    //if (m_material_widget.accepted()) {
    //    rebuildViewer();
    //}
    return m_done;
}
