//
// Created by Thomas Ibanez on 09.12.20.
//

#ifndef ICE_PROJECT_H
#define ICE_PROJECT_H

#include <string>
#include <vector>
#include <Scene/Scene.h>
#include <Assets/AssetBank.h>
#include <json/json.h>

using json = nlohmann::json;

namespace ICE {
    class Project {
    public:
        bool CreateDirectories();

        Project(const std::string &baseDirectory, const std::string &name);

        const std::string &getBaseDirectory() const;
        const std::string &getName() const;

        void writeToFile(Camera* editorCamera);
        void loadFromFile();
        void writeMaterialFile(const std::string& mtlName, const Material& mtl);
        Material* loadMaterial(const std::string& file);
        void copyAssetFile(const std::string& folder, const std::string& assetName, const std::string& src);
        bool renameAsset(const std::string& oldName, const std::string& newName);

        std::vector<Scene> &getScenes();
        void setScenes(const std::vector<Scene> &scenes);

        AssetBank* getAssetBank();
        void setAssetBank(const AssetBank &assetBank);

        void addScene(Scene& scene);

        static json dumpVec3(const Eigen::Vector3f& v);
        static json dumpVec4(const Eigen::Vector4f& v);

        const Eigen::Vector3f &getCameraPosition() const;

        const Eigen::Vector3f &getCameraRotation() const;

    private:

        std::vector<std::string> getFilesInDir(const std::string& folder);

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
        AssetBank assetBank;
        Eigen::Vector3f cameraPosition, cameraRotation;
    };
}


#endif //ICE_PROJECT_H
