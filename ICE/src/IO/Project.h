//
// Created by Thomas Ibanez on 09.12.20.
//

#ifndef ICE_PROJECT_H
#define ICE_PROJECT_H

#include <string>
#include <vector>
#include <Scene/Scene.h>
#include <Assets/AssetBank.h>

namespace ICE {
    class Project {
    public:
        bool CreateDirectories();

        Project(const std::string &baseDirectory, const std::string &name);

        const std::string &getBaseDirectory() const;
        const std::string &getName() const;

        void writeToFile();
        void loadFromFile();

        void setBaseDirectory(const std::string &baseDirectory);

        void setName(const std::string &name);

        std::vector<Scene> &getScenes();

        void setScenes(const std::vector<Scene> &scenes);

        AssetBank* getAssetBank();

        void setAssetBank(const AssetBank &assetBank);

        static std::string Vec3ToString(const Eigen::Vector3f& v) {
           return "["+std::to_string(v.x())+","+std::to_string(v.y())+","+std::to_string(v.z())+"]";
        }

        static std::string Vec4ToString(const Eigen::Vector4f& v) {
            return "["+std::to_string(v.x())+","+std::to_string(v.y())+","+std::to_string(v.z())+","+std::to_string(v.w())+"]";
        }

        static Eigen::Vector3f FromString(const std::string& str) {
            Eigen::Vector3f vec;
            int b = str.find("[")+1;
            int e = str.find(",");
            std::string x = str.substr(b, e);
            vec.x() = atof(x.c_str());
            b = e+1;
            e = str.find(",", b);
            std::string y = str.substr(b, e);
            vec.y() = atof(y.c_str());
            b = e+1;
            e = str.find(",", b);
            std::string z = str.substr(b, e);
            vec.z() = atof(z.c_str());
            return vec;
        }
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
