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
#include <Scene/Scene.h>
#include <Util/OBJLoader.h>

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
        scenes.push_back(Scene("MainScene"));
        return true;
    }

    const std::string &Project::getBaseDirectory() const {
        return baseDirectory;
    }

    const std::string &Project::getName() const {
        return name;
    }

    void Project::writeToFile(Camera* editorCamera) {
        std::ofstream outstream;
        outstream.open(baseDirectory + "/" + name + "/" + name+ ".ice");
        json j;

        j["camera_position"] = dumpVec3(editorCamera->getPosition());
        j["camera_rotation"] = dumpVec3(editorCamera->getRotation());

        std::vector<std::string> strvec;
        for(auto s : scenes) {
            strvec.push_back(s.getName());
        }
        j["scenes"] = strvec;
        strvec.clear();

        for(auto m : assetBank.getMeshes()) {
            if(m.first.find("__ice__") == std::string::npos) {
                strvec.push_back(m.first);
            }
        }
        j["meshes"] = strvec;
        strvec.clear();

        for(auto m : assetBank.getMaterials()) {
            if(m.first.find("__ice__") == std::string::npos) {
                strvec.push_back(m.first);
                writeMaterialFile(m.first, *m.second);
            }
        }
        j["materials"] = strvec;
        strvec.clear();

        for(auto m : assetBank.getShaders()) {
            if (m.first.find("__ice__") == std::string::npos) {
                strvec.push_back(m.first);
            }
        }
        j["shaders"] = strvec;
        strvec.clear();

        for(auto m : assetBank.getTextures()) {
            if(m.first.find("__ice__") == std::string::npos) {
                strvec.push_back(m.first);
            }
        }
        j["textures"] = strvec;
        outstream << j.dump(4);
        outstream.close();

        for(auto s : scenes) {
            outstream.open(baseDirectory + "/" + name + "/Scenes/" + s.getName() + ".ics");
            j.clear();

            j["name"] = s.getName();
            json entities = json::array();
            for(auto e : s.getEntities()) {
                json entity;
                entity["name"] = s.idByEntity(e);
                entity["parent"] = s.getParent(s.idByEntity(e));
                if(e->hasComponent<RenderComponent>()) {
                    RenderComponent rc = *e->getComponent<RenderComponent>();
                    json renderjson;
                    renderjson["mesh"] = assetBank.getName(rc.getMesh());
                    renderjson["material"] = assetBank.getName(rc.getMaterial());
                    renderjson["shader"] = assetBank.getName(rc.getShader());
                    entity["renderComponent"] = renderjson;
                }
                if(e->hasComponent<TransformComponent>()) {
                    TransformComponent tc = *e->getComponent<TransformComponent>();
                    json transformjson;
                    transformjson["position"] = dumpVec3(*tc.getPosition());
                    transformjson["rotation"] = dumpVec3(*tc.getRotation());
                    transformjson["scale"] = dumpVec3(*tc.getScale());
                    entity["transformComponent"] = transformjson;
                }
                if(e->hasComponent<LightComponent>()) {
                    LightComponent lc = *e->getComponent<LightComponent>();
                    json lightjson;
                    lightjson["color"] = dumpVec3(lc.getColor());
                    lightjson["type"] = lc.getType();
                    entity["lightComponent"] = lightjson;
                }
                entities.push_back(entity);
            }
            j["entities"] = entities;
            outstream << j.dump(4);
            outstream.close();
        }
    }

    void Project::loadFromFile() {
        std::ifstream infile = std::ifstream(baseDirectory + "/" + name + "/" + name+ ".ice");
        json j;
        infile >> j;
        infile.close();

        std::vector<std::string> sceneNames = j["scenes"];
        std::vector<std::string> meshesNames = j["meshes"];
        std::vector<std::string> materialNames = j["materials"];
        std::vector<std::string> shaderNames = j["shaders"];
        std::vector<std::string> textureNames = j["textures"];

        cameraPosition = parseVec3(j["camera_position"]);
        cameraRotation = parseVec3(j["camera_rotation"]);

        std::string path = baseDirectory + "/" + name + "/Assets/Textures/";
        std::vector<std::string> files;
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string sp = entry.path().string();
            files.push_back(sp.substr(sp.find_last_of("/")+1));
        }

        for(auto m : textureNames) {
            for(auto file : files) {
                if(file.find(m) != std::string::npos) {
                    assetBank.addTexture(m, Texture2D::Create(path+file));
                    break;
                }
            }
        }

        files.clear();
        path = baseDirectory + "/" + name + "/Assets/Meshes/";
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string sp = entry.path().string();
            files.push_back(sp.substr(sp.find_last_of("/")+1));
        }

        for(auto m : meshesNames) {
            for(auto file : files) {
                if(file.find(m) != std::string::npos) {
                    assetBank.addMesh(m, OBJLoader::loadFromOBJ(path+file));
                    break;
                }
            }
        }

        for(auto m : materialNames) {
            assetBank.addMaterial(m, loadMaterial(m));
        }

        for(auto s : sceneNames) {
            infile = std::ifstream(baseDirectory + "/" + name + "/Scenes/" + s + ".ics");
            json scenejson;
            infile >> scenejson;
            infile.close();

            Scene scene = Scene(scenejson["name"]);
            for(json jentity : scenejson["entities"]) {
                Entity* e = new Entity();
                if(!jentity["transformComponent"].is_null()) {
                    json tj = jentity["transformComponent"];
                    TransformComponent* tc = new TransformComponent(parseVec3(tj["position"]),parseVec3(tj["rotation"]),parseVec3(tj["scale"]));
                    e->addComponent(tc);
                }
                if(!jentity["renderComponent"].is_null()) {
                    json rj = jentity["renderComponent"];
                    RenderComponent* rc = new RenderComponent(assetBank.getMesh(rj["mesh"]),assetBank.getMaterial(rj["material"]),assetBank.getShader(rj["shader"]));
                    e->addComponent(rc);
                }
                if(!jentity["lightComponent"].is_null()) {
                    json lj = jentity["lightComponent"];
                    LightComponent* lc = new LightComponent(PointLight, parseVec3(lj["color"]));
                    e->addComponent(lc);
                }
                scene.addEntity(jentity["parent"], jentity["name"], e);
            }
            scenes.push_back(scene);
        }
    }

    void Project::writeMaterialFile(const std::string& mtlName, const Material& mtl) {
        std::ofstream infile = std::ofstream(baseDirectory + "/" + name + "/Assets/Materials/" + mtlName + ".icm");
        json j;
        j["type"] = "phong";
        j["albedo"] = dumpVec3(mtl.getAlbedo());
        j["specular"] = dumpVec3(mtl.getSpecular());
        j["ambient"] = dumpVec3(mtl.getAmbient());
        j["alpha"] = mtl.getAlpha();
        j["diffuseMap"] = assetBank.getName(mtl.getDiffuseMap());
        j["specularMap"] = assetBank.getName(mtl.getSpecularMap());
        j["ambientMap"] = assetBank.getName(mtl.getAmbientMap());
        j["normalMap"] = assetBank.getName(mtl.getNormalMap());
        infile << j.dump(4);
        infile.close();
    }

    Material* Project::loadMaterial(const std::string& mtlName) {
        json j;
        std::ifstream infile = std::ifstream(baseDirectory + "/" + name + "/Assets/Materials/" + mtlName + ".icm");
        infile >> j;
        infile.close();

        Material* mtl = new Material();
        if(j["type"] == "phong") {
            mtl->setAlbedo(parseVec3(j["albedo"]));
            mtl->setSpecular(parseVec3(j["specular"]));
            mtl->setAmbient(parseVec3(j["ambient"]));
            mtl->setAlpha(j["alpha"]);
            if(j["diffuseMap"] != "null") {
                mtl->setDiffuseMap(assetBank.getTexture(j["diffuseMap"]));
            }
            if(j["specularMap"] != "null") {
                mtl->setSpecularMap(assetBank.getTexture(j["specularMap"]));
            }
            if(j["ambientMap"] != "null") {
                mtl->setAmbientMap(assetBank.getTexture(j["ambientMap"]));
            }
            if(j["normalMap"] != "null") {
                mtl->setNormalMap(assetBank.getTexture(j["normalMap"]));
            }
        }
        return mtl;
    }

    void Project::copyAssetFile(const std::string& folder, const std::string& assetName, const std::string &src) {
        std::ifstream  srcStream(src, std::ios::binary);
        std::string dst = baseDirectory + "/" + name + "/Assets/"+folder+"/" + assetName + src.substr(src.find_last_of("."));
        std::ofstream  dstStream(dst, std::ios::binary);

        dstStream << srcStream.rdbuf();
        dstStream.flush();
        srcStream.close();
        dstStream.close();
    }

    bool Project::renameAsset(const std::string& oldName, const std::string& newName) {
        if(newName == "") {
            return false;
        }
        if(assetBank.renameAsset(oldName, newName)) {
            std::string path = baseDirectory + "/" + name + "/Assets/";
            std::string searchDir[3] = {"Textures/", "Meshes/", "Materials/"};
            for(int i = 0; i < 3; i++) {
                for(auto file : getFilesInDir(path+searchDir[i])) {
                    if(file.substr(0,file.find_last_of(".")) == oldName) {
                        if(rename((path+searchDir[i]+file).c_str(), (path+searchDir[i]+(newName+file.substr(file.find_last_of(".")))).c_str()) == 0) {
                            return true;
                        } else {
                            assetBank.renameAsset(newName, oldName);
                        }
                    }
                }
            }
        }
        return false;
    }

    std::vector<std::string> Project::getFilesInDir(const std::string &folder) {
        std::vector<std::string> files;
        for (const auto& entry : fs::directory_iterator(folder)) {
            std::string sp = entry.path().string();
            files.push_back(sp.substr(sp.find_last_of("/")+1));
        }
        return files;
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

    void Project::addScene(Scene &scene) {
        scenes.push_back(scene);
    }

    json Project::dumpVec3(const Eigen::Vector3f &v) {
        json r;
        r["x"] = v.x();
        r["y"] = v.y();
        r["z"] = v.z();
        return r;
    }

    json Project::dumpVec4(const Eigen::Vector4f &v) {
        json r;
        r["x"] = v.x();
        r["y"] = v.y();
        r["z"] = v.z();
        r["w"] = v.w();
        return r;
    }

    Eigen::Vector3f Project::parseVec3(const json &src) {
        return Eigen::Vector3f(src["x"],src["y"],src["z"]);
    }

    Eigen::Vector4f Project::parseVec4(const json &src) {
        return Eigen::Vector4f(src["x"],src["y"],src["z"],src["w"]);
    }

    const Eigen::Vector3f &Project::getCameraPosition() const {
        return cameraPosition;
    }

    const Eigen::Vector3f &Project::getCameraRotation() const {
        return cameraRotation;
    }
}