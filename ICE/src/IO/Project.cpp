//
// Created by Thomas Ibanez on 09.12.20.
//

#include "Project.h"
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#else
  error "Missing the <filesystem> header."
#endif

#include <iostream>
#include <fstream>
#include <Scene/RenderComponent.h>
#include <Scene/TransformComponent.h>
#include <Scene/LightComponent.h>
#include <Scene/Entity.h>

namespace ICE {
    Project::Project(const std::string &baseDirectory, const std::string &name) : baseDirectory(baseDirectory),
                                                                                  name(name), scenes(std::vector<Scene>()),
                                                                                  assetBank(AssetBank()) {}

    bool Project::CreateDirectories() {
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Meshes");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Materials");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Shaders");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Textures");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Scripts");
        fs::create_directories(baseDirectory + "/" + name + "/Scenes");
        scenes.push_back(Scene("NewScene0"));
        return true;
    }

    const std::string &Project::getBaseDirectory() const {
        return baseDirectory;
    }

    const std::string &Project::getName() const {
        return name;
    }

    void Project::writeToFile() {
        std::ofstream outstream;
        outstream.open(baseDirectory + "/" + name + "/" + name+ ".ice");
        outstream << "Scenes\n";
        for(auto s : scenes) {
            outstream << " - " << s.getName() << "\n";
        }
        outstream << "Meshes\n";
        for(auto m : assetBank.getMeshes()) {
            if(m.first.find("__ice__") == std::string::npos) {
                outstream << " - " << m.first << "\n";
            }
        }

        outstream << "Materials\n";
        for(auto m : assetBank.getMaterials()) {
            if(m.first.find("__ice__") == std::string::npos) {
                outstream << " - " << m.first << "\n";
            }
        }

        outstream << "Shaders\n";
        for(auto m : assetBank.getShaders()) {
            if(m.first.find("__ice__") == std::string::npos) {
                outstream << " - " << m.first << "\n";
            }
        }

        outstream << "Textures\n";
        for(auto m : assetBank.getTextures()) {
            if(m.first.find("__ice__") == std::string::npos) {
                outstream << " - " << m.first << "\n";
            }
        }
        outstream.close();

        for(auto s : scenes) {
            outstream.open(baseDirectory + "/" + name + "/Scenes/" + s.getName() + ".ics");
            outstream << "Name\n";
            outstream << " - " << s.getName() << "\n";
            outstream << "Entities\n";
            for(auto e : s.getEntities()) {
                outstream << " + " << s.idByEntity(e) << " " << s.getParent(s.idByEntity(e)) << "\n";
                if(e->hasComponent<RenderComponent>()) {
                    RenderComponent rc = *e->getComponent<RenderComponent>();
                    outstream << "RenderComponent\n";
                    outstream << " - " << "Mesh " << assetBank.getName(rc.getMesh()) << "\n";
                    outstream << " - " << "Material " << assetBank.getName(rc.getMaterial()) << "\n";
                    outstream << " - " << "Shader " << assetBank.getName(rc.getShader()) << "\n";
                }
                if(e->hasComponent<TransformComponent>()) {
                    TransformComponent tc = *e->getComponent<TransformComponent>();
                    outstream << "TransformComponent\n";
                    outstream << " - " << "Position " << Vec3ToString(*tc.getPosition()) << "\n";
                    outstream << " - " << "Rotation " << Vec3ToString(*tc.getRotation()) << "\n";
                    outstream << " - " << "Scale " << Vec3ToString(*tc.getScale()) << "\n";
                }
                if(e->hasComponent<LightComponent>()) {
                    LightComponent lc = *e->getComponent<LightComponent>();
                    outstream << "LightComponent\n";
                    outstream << " - " << "Color " << Vec3ToString(lc.getColor()) << "\n";
                    outstream << " - " << "Type " << lc.getType() << "\n";
                }
            }
            outstream.close();
        }
    }

    void Project::loadFromFile() {
        std::ifstream infile = std::ifstream(baseDirectory + "/" + name + "/" + name+ ".ice");

        auto stage = LoadStage::Scenes;
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            if(line == "Scenes") {
                stage = LoadStage::Scenes;
                continue;
            } else if(line == "Meshes") {
                stage = LoadStage::Meshes;
                continue;
            } else if(line == "Materials") {
                stage = LoadStage::Materials;
                continue;
            } else if(line == "Shaders") {
                stage = LoadStage::Shaders;
                continue;
            } else if(line == "Textures") {
                stage = LoadStage::Textures;
                continue;
            }

            switch(stage) {
                case Scenes:
                    scenes.push_back(Scene(line.substr(3)));
                    break;
                case Meshes:
                    break;
                case Materials:
                    break;
                case Shaders:
                    break;
                case Textures:
                    break;
            }
        }
        infile.close();

        auto sls = SceneLoadStage::NameStage;
        auto els = EntityLoadStage::TransformComponentStage;
        infile = std::ifstream(baseDirectory + "/" + name + "/Scenes/" + scenes[0].getName() + ".ics");
        Entity e;
        std::string uid = "";
        std::string parent;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            if(line == "Name") {
                sls = NameStage;
                continue;
            } else if(line == "Entities") {
                sls = EntityStage;
                continue;
            }

            if(sls == NameStage) {
                continue; //skip
            }
            if(sls == EntityStage) {
                if(line.find("+") != std::string::npos) {
                    if(uid != "") {
                        Entity* ne = new Entity();
                        *ne = e;
                        scenes[0].addEntity(parent, uid, ne);
                        e = Entity();
                    }
                    std::string skipped = line.substr(3);
                    uid = skipped.substr(0, skipped.find(" "));
                    parent = skipped.substr(skipped.find(" ") + 1);
                    continue;
                } else if(line == "RenderComponent") {
                    els = RenderComponentStage;
                    e.addComponent(new RenderComponent(nullptr, nullptr, nullptr));
                    continue;
                } else if(line == "TransformComponent") {
                    els = TransformComponentStage;
                    e.addComponent(new TransformComponent());
                    continue;
                } else if(line == "LightComponent") {
                    els = LightComponentStage;
                    e.addComponent(new LightComponent(PointLight, Eigen::Vector3f()));
                    continue;
                }

                switch(els) {
                    case RenderComponentStage:
                        if(line.find("Mesh") != std::string::npos) {
                            e.getComponent<RenderComponent>()->setMesh(assetBank.getMesh(line.substr(8)));
                        } else if(line.find("Material") != std::string::npos) {
                            e.getComponent<RenderComponent>()->setMaterial(assetBank.getMaterial(line.substr(12)));
                        } else if(line.find("Shader") != std::string::npos) {
                            e.getComponent<RenderComponent>()->setShader(assetBank.getShader(line.substr(10)));
                        }
                        break;
                    case TransformComponentStage:
                        if(line.find("Position") != std::string::npos) {
                            *e.getComponent<TransformComponent>()->getPosition() = FromString(line);
                        } else if(line.find("Rotation") != std::string::npos) {
                            *e.getComponent<TransformComponent>()->getRotation() = FromString(line);
                        } else if(line.find("Scale") != std::string::npos) {
                            *e.getComponent<TransformComponent>()->getScale() = FromString(line);
                        }
                        break;
                    case LightComponentStage:
                        if(line.find("Color") != std::string::npos) {
                            Eigen::Vector3f v = FromString(line);
                            e.getComponent<LightComponent>()->getColor().x() = v.x();
                            e.getComponent<LightComponent>()->getColor().y() = v.y();
                            e.getComponent<LightComponent>()->getColor().z() = v.z();
                        }
                        break;
                }
            }

        }
        Entity* ne = new Entity();
        *ne = e;
        scenes[0].addEntity(parent, uid, ne);
        infile.close();
    }

    void Project::setBaseDirectory(const std::string &baseDirectory) {
        Project::baseDirectory = baseDirectory;
    }

    void Project::setName(const std::string &name) {
        Project::name = name;
    }

    std::vector<Scene> &Project::getScenes() {
        return scenes;
    }

    void Project::setScenes(const std::vector<Scene> &scenes) {
        Project::scenes = scenes;
    }

    AssetBank* Project::getAssetBank() {
        return &assetBank;
    }

    void Project::setAssetBank(const AssetBank &assetBank) {
        Project::assetBank = assetBank;
    }
}