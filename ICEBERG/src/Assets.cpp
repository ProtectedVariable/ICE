#include "Assets.h"

Assets::Assets(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {
    auto entries = engine->getAssetBank()->getAllEntries();
    for (const auto& entry : entries) {

    }
    ui.addAssets({"Mesh", {"Mesh0", "Mesh1", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "Mesh2", "MeshN"}});
    AssetView sub_1 = {"Subfolder1", {"M1"}, {}};
    ui.addAssets({"Materials", {"Material0", "Material1", "Mesh2"}, {std::make_shared<AssetView>(sub_1)}});
}

bool Assets::update() {
    ui.render();
    return m_done;
}
