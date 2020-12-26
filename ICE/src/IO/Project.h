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

        void writeToFile();
        void loadFromFile();

        std::vector<Scene> &getScenes();
        void setScenes(const std::vector<Scene> &scenes);

        AssetBank* getAssetBank();
        void setAssetBank(const AssetBank &assetBank);

        static json dumpVec3(const Eigen::Vector3f& v);
        static json dumpVec4(const Eigen::Vector4f& v);

        static Eigen::Vector3f parseVec3(const json& src);
        static Eigen::Vector4f parseVec4(const json& src);

    private:

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
    };
}


#endif //ICE_PROJECT_H
