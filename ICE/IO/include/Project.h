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
#include <string>
#include <vector>
#include <Scene.h>
#include <AssetBank.h>
#include <json/json.h>
#include <Camera.h>

using json = nlohmann::json;

namespace ICE {
    class Project {
    public:
        bool CreateDirectories();

        Project(const std::string &baseDirectory, const std::string &name);

        std::string getBaseDirectory() const;
        std::string getName() const;

        void writeToFile(std::shared_ptr<Camera> editorCamera);
        void loadFromFile();
        void writeMaterialFile(const std::string& mtlName, const Material& mtl);
        void copyAssetFile(const std::string& folder, const std::string& assetName, const std::string& src);
        bool renameAsset(const AssetPath& oldName, const AssetPath& newName);

        std::vector<Scene> &getScenes();
        void setScenes(const std::vector<Scene> &scenes);

        std::shared_ptr<AssetBank> getAssetBank();
        void setAssetBank(const std::shared_ptr<AssetBank>& assetBank);

        void addScene(Scene& scene);

        static json dumpVec3(const Eigen::Vector3f& v);
        static json dumpVec4(const Eigen::Vector4f& v);

        const Eigen::Vector3f &getCameraPosition() const;

        const Eigen::Vector3f &getCameraRotation() const;

    private:

        std::vector<std::string> getFilesInDir(const std::string& folder);

        template<typename T>
        void loadAssetsOfType(std::string basepath, json names) {
            std::vector<std::string> files;
            std::string typeFolder = AssetPath::WithTypePrefix<T>("").toString();
            for (const auto& entry : fs::directory_iterator(basepath+typeFolder)) {
                std::string sp = entry.path().string();
                files.push_back(sp.substr(sp.find_last_of("/")+1));
            }
            for(auto m : names.items()) {
                AssetPath assetPath = AssetPath(m.value().begin().key());
                for(auto file : files) {
                    if(file.substr(0,file.find_last_of(".")) == assetPath.getName()) {
                        assetBank->addAssetWithSpecificUID<T>(m.value().begin().key(), {(basepath+typeFolder+file)}, m.value().begin().value());
                        break;
                    }
                }
            }
        }

        enum LoadStage {
            Scenes, Meshes, Materials, Shaders, Textures
        };

        enum SceneLoadStage {
            NameStage, EntityStage
        };

        enum EntityLoadStage {
            RenderComponentStage, TransformComponentStage, LightComponentStage
        };

        std::string baseDirectory;
        std::string name;
        std::vector<Scene> scenes;
        std::shared_ptr<AssetBank> assetBank;
        Eigen::Vector3f cameraPosition, cameraRotation;

    };
}


#endif //ICE_PROJECT_H
