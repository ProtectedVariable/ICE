//
// Created by Thomas Ibanez on 09.12.20.
//

#ifndef ICE_PROJECT_H
#define ICE_PROJECT_H
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif
#include <AssetBank.h>
#include <Camera.h>
#include <GPURegistry.h>
#include <Scene.h>
#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace ICE {
class Project {
   public:
    bool CreateDirectories();

    Project(const fs::path& m_base_directory, const std::string& name);

    fs::path getBaseDirectory() const;
    std::string getName() const;

    void writeToFile(const std::shared_ptr<Camera>& editorCamera);
    void loadFromFile();
    void copyAssetFile(const fs::path& folder, const std::string& assetName, const fs::path& src);
    bool renameAsset(const AssetPath& oldName, const AssetPath& newName);

    // Import a model file into the project: copies `src` into the project's Models folder and
    // registers it in the asset bank under `name`. Folds the copyAssetFile + addAsset + path
    // reconstruction that callers used to spell out by hand. Returns the new asset's UID.
    AssetUID importModel(const std::string& name, const fs::path& src);

    // Resolve a bank UID by name for a given asset kind, hiding AssetPath::WithTypePrefix from
    // gameplay/tools code. Return NO_ASSET_ID if no such asset is registered.
    AssetUID mesh(const std::string& name) const;
    AssetUID material(const std::string& name) const;
    AssetUID model(const std::string& name) const;

    std::vector<std::shared_ptr<Scene>> getScenes();
    void setScenes(const std::vector<std::shared_ptr<Scene>>& scenes);

    std::shared_ptr<AssetBank> getAssetBank();
    void setAssetBank(const std::shared_ptr<AssetBank>& assetBank);
    std::shared_ptr<GPURegistry> getGPURegistry();

    void addScene(const Scene& scene);
    void setCurrentScene(const std::shared_ptr<Scene>& scene);
    std::shared_ptr<Scene> getCurrentScene() const;

    // Create a scene, add it, make it current, and -- if this project is attached to an engine --
    // activate it (set up its runtime systems) so it is ready to render. Returns the owned scene.
    Scene& createScene(const std::string& name);

    // Installed by the engine when it adopts the project (see ICEEngine::newProject); createScene
    // invokes it so a new scene gets its runtime systems. Runtime-only, never serialized.
    void setSceneActivator(const std::function<void(const std::shared_ptr<Scene>&)>& activator);

    static json dumpVec3(const Eigen::Vector3f& v);
    static json dumpVec4(const Eigen::Vector4f& v);

    const Eigen::Vector3f& getCameraPosition() const;

    const Eigen::Vector3f& getCameraRotation() const;

   private:
    json dumpAsset(AssetUID uid, const std::shared_ptr<Asset>& asset);

    std::vector<std::string> getFilesInDir(const fs::path& folder);

    template<typename T>
    void loadAssetsOfType(const json& assets) {
        for (const auto& asset : assets) {
            AssetUID uid = asset["uid"];
            std::string asset_path = asset["bank_path"];
            std::vector<fs::path> sources;
            for (const auto& entry : asset["sources"]) {
                std::string source = entry;
                sources.push_back(m_base_directory / source);
            }
            m_asset_bank->addAssetWithSpecificUID<T>(asset_path, sources, uid);
        }
    }

    enum LoadStage { Scenes, Meshes, Materials, Shaders, Textures };

    enum SceneLoadStage { NameStage, EntityStage };

    enum EntityLoadStage { RenderComponentStage, TransformComponentStage, LightComponentStage };

    fs::path m_base_directory;
    fs::path m_scenes_directory;
    fs::path m_models_directory;
    fs::path m_meshes_directory;
    fs::path m_materials_directory;
    fs::path m_shaders_directory;
    fs::path m_textures_directory;
    fs::path m_cubemaps_directory;
    std::string m_name;

    std::vector<std::shared_ptr<Scene>> m_scenes;
    std::shared_ptr<Scene> m_current_scene;
    std::function<void(const std::shared_ptr<Scene>&)> m_scene_activator;

    std::shared_ptr<AssetBank> m_asset_bank;
    std::shared_ptr<GPURegistry> m_gpu_registry;

    // Custom-asset entries from the "assets" section whose type/prefix couldn't be resolved at load
    // time (the providing plugin wasn't registered). Kept verbatim and re-emitted on save so a
    // missing plugin doesn't silently drop the user's assets.
    std::vector<json> m_unknown_assets;

    Eigen::Vector3f cameraPosition, cameraRotation;
};
}  // namespace ICE

#endif  //ICE_PROJECT_H
