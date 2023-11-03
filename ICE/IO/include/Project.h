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
#include <Scene.h>
#include <json/json.h>

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

    std::vector<Scene>& getScenes();
    void setScenes(const std::vector<Scene>& scenes);

    std::shared_ptr<AssetBank> getAssetBank();
    void setAssetBank(const std::shared_ptr<AssetBank>& assetBank);

    void addScene(Scene& scene);

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
            assetBank->addAssetWithSpecificUID<T>(asset_path, sources, uid);
        }
    }

    enum LoadStage { Scenes, Meshes, Materials, Shaders, Textures };

    enum SceneLoadStage { NameStage, EntityStage };

    enum EntityLoadStage { RenderComponentStage, TransformComponentStage, LightComponentStage };

    fs::path m_base_directory;
    fs::path m_scenes_directory;
    fs::path m_meshes_directory;
    fs::path m_materials_directory;
    fs::path m_shaders_directory;
    fs::path m_textures_directory;
    fs::path m_cubemaps_directory;
    std::string name;
    std::vector<Scene> scenes;
    std::shared_ptr<AssetBank> assetBank;
    Eigen::Vector3f cameraPosition, cameraRotation;
};
}  // namespace ICE

#endif  //ICE_PROJECT_H
