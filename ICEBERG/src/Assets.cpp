#include "Assets.h"

#include <ForwardRenderer.h>
#include <PerspectiveCamera.h>

Assets::Assets(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_g_factory(g_factory),
      m_material_widget(engine) {
    rebuildViewer();
    ui.registerCallback("material_edit", [this](std::string name) { m_material_widget.open(m_engine->getAssetBank()->getUID("Materials/" + name)); });
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

    shader->bind();
    shader->loadMat4("projection", camera->getProjection());
    shader->loadMat4("view", camera->lookThrough());

    ICE::GeometryPass pass(m_engine->getApi(), m_engine->getGraphicsFactory(), {256, 256, 1});
    std::vector<ICE::RenderCommand> cmds;
    for (const auto& mesh : model->getMeshes()) {
        cmds.push_back(ICE::RenderCommand{.mesh = mesh, .material = material, .shader = shader, .model_matrix = Eigen::Matrix4f::Identity()});
    }
    pass.submit(&cmds);
    pass.execute();

    return static_cast<char*>(0) + pass.getResult()->getTexture();
}

void Assets::rebuildViewer() {
    ui.reset();

    auto entries = m_engine->getAssetBank()->getAllEntries();
    //Fill asset browser
    std::unordered_map<std::string, std::shared_ptr<AssetView>> asset_roots;

    for (const auto& entry : entries) {
        auto root = entry.path.getPath()[0];
        if (!asset_roots.contains(root)) {
            asset_roots.try_emplace(root, std::make_shared<AssetView>(AssetView{root, {}, {}}));
        }
        std::shared_ptr<AssetView> parent = asset_roots[root];
        for (const auto& subpath : entry.path.getPath()) {
            if (subpath == root) {
                continue;
            }
            for (const auto& subfolder : parent->subfolders) {
                if (subfolder->folder_name == subpath) {
                    parent = subfolder;
                    break;
                }
            }
            if (parent == asset_roots[root]) {
                auto new_folder = std::make_shared<AssetView>(AssetView{subpath, {}, {}});
                parent->subfolders.push_back(new_folder);
                parent = new_folder;
            }
        }

        parent->assets.emplace_back(entry.path.getName(), createThumbnail(entry));
    }

    for (const auto& [name, assetview] : asset_roots) {
        ui.addAssets(assetview);
    }
}

bool Assets::update() {
    ui.render();
    m_material_widget.render();
    if (m_material_widget.accepted()) {
        rebuildViewer();
    }
    return m_done;
}
