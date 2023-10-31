#include "Assets.h"

Assets::Assets(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    auto entries = engine->getAssetBank()->getAllEntries();

    //Fill asset browser
    std::unordered_map<std::string, std::shared_ptr<AssetView>> asset_roots;

    for (const auto& entry : entries) {
        auto& root = entry.path.getPath().front();
        if (!asset_roots.contains(entry.path.getPath().front())) {
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

        parent->assets_names.push_back(entry.path.getName());
    }

    for (const auto& [name, assetview] : asset_roots) {
        ui.addAssets(assetview);
    }
}

bool Assets::update() {
    ui.render();
    return m_done;
}
