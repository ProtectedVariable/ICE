//
// Created by Thomas Ibanez on 09.12.20.
//

#include "Project.h"

#include <Entity.h>
#include <JsonParser.h>
#include <LightComponent.h>
#include <OpenGLFactory.h>
#include <RenderComponent.h>
#include <Scene.h>
#include <TransformComponent.h>

#include <fstream>
#include <iostream>
#define FSEP ("/")
#define ICE_PROJECT_EXT ".ice"

#define ICE_ASSET_FOLDER FSEP + "Assets"
#define ICE_ASSET_MESH_FOLDER ICE_ASSET_FOLDER + FSEP + "Meshes"
#define ICE_ASSET_MATERIAL_FOLDER ICE_ASSET_FOLDER + FSEP + "Materials"
#define ICE_ASSET_SHADERS_FOLDER ICE_ASSET_FOLDER + FSEP + "Shaders"
#define ICE_ASSET_TEXTURES_FOLDER ICE_ASSET_FOLDER + FSEP + "Textures"
#define ICE_ASSET_CUBEMAPS_FOLDER ICE_ASSET_FOLDER + FSEP + "CubeMaps"
#define ICE_ASSET_SCRIPTS_FOLDER ICE_ASSET_FOLDER + FSEP + "Scripts"

#define ICE_SCENES_FOLDER FSEP + "Scenes"

namespace ICE {
Project::Project(const std::string &baseDirectory, const std::string &name)
    : baseDirectory(baseDirectory),
      name(name),
      scenes(std::vector<Scene>()),
      assetBank(AssetBank(std::make_shared<OpenGLFactory>())) {
    cameraPosition.setZero();
    cameraRotation.setZero();
}

bool Project::CreateDirectories() {
    fs::create_directories(baseDirectory + FSEP + name + ICE_ASSET_MESH_FOLDER);
    fs::create_directories(baseDirectory + FSEP + name + ICE_ASSET_MATERIAL_FOLDER);
    fs::create_directories(baseDirectory + FSEP + name + ICE_ASSET_SHADERS_FOLDER);
    fs::create_directories(baseDirectory + FSEP + name + ICE_ASSET_TEXTURES_FOLDER);
    fs::create_directories(baseDirectory + FSEP + name + ICE_ASSET_CUBEMAPS_FOLDER);
    fs::create_directories(baseDirectory + FSEP + name + ICE_ASSET_SCRIPTS_FOLDER);
    fs::create_directories(baseDirectory + FSEP + name + ICE_SCENES_FOLDER);
    scenes.push_back(Scene("MainScene", new Registry()));
    assetBank.fillWithDefaults();
    return true;
}

const std::string &Project::getBaseDirectory() const {
    return baseDirectory;
}

const std::string &Project::getName() const {
    return name;
}

void Project::writeToFile(Camera *editorCamera) {
    std::ofstream outstream;
    outstream.open(baseDirectory + FSEP + name + FSEP + name + ICE_PROJECT_EXT);
    json j;

    j["camera_position"] = dumpVec3(editorCamera->getPosition());
    j["camera_rotation"] = dumpVec3(editorCamera->getRotation());

    std::vector<json> vec;
    for (auto s : scenes) {
        vec.push_back(s.getName());
    }
    j["scenes"] = vec;
    vec.clear();

    for (auto m : assetBank.getAll<Mesh>()) {
        if (assetBank.getName(m.first).getName().find(ICE_ASSET_PREFIX) == std::string::npos) {
            json tmp;
            tmp[assetBank.getName(m.first).toString()] = m.first;
            vec.push_back(tmp);
        }
    }
    j["meshes"] = vec;
    vec.clear();

    for (auto m : assetBank.getAll<Material>()) {
        if (assetBank.getName(m.first).getName().find(ICE_ASSET_PREFIX) == std::string::npos) {
            json tmp;
            tmp[assetBank.getName(m.first).toString()] = m.first;
            vec.push_back(tmp);
            writeMaterialFile(assetBank.getName(m.first).getName(), *m.second);
        }
    }
    j["materials"] = vec;
    vec.clear();

    for (auto m : assetBank.getAll<Shader>()) {
        if (assetBank.getName(m.first).getName().find(ICE_ASSET_PREFIX) == std::string::npos) {
            json tmp;
            tmp[assetBank.getName(m.first).toString()] = m.first;
            vec.push_back(tmp);
        }
    }
    j["shaders"] = vec;
    vec.clear();

    for (auto m : assetBank.getAll<Texture2D>()) {
        if (assetBank.getName(m.first).getName().find("__ice__") == std::string::npos) {
            json tmp;
            tmp[assetBank.getName(m.first).toString()] = m.first;
            vec.push_back(tmp);
        }
    }
    j["textures2D"] = vec;
    vec.clear();

    for (auto m : assetBank.getAll<TextureCube>()) {
        if (assetBank.getName(m.first).getName().find("__ice__") == std::string::npos) {
            json tmp;
            tmp[assetBank.getName(m.first).toString()] = m.first;
            vec.push_back(tmp);
        }
    }
    j["cubeMaps"] = vec;
    outstream << j.dump(4);
    outstream.close();

    for (auto s : scenes) {
        outstream.open(baseDirectory + "/" + name + "/Scenes/" + s.getName() + ".ics");
        j.clear();

        j["name"] = s.getName();
        json entities = json::array();
        for (auto e : s.getRegistry()->getEntities()) {
            json entity;
            entity["name"] = e;
            //entity["parent"] = s.getParent(e);

            if (s.getRegistry()->entityHasComponent<RenderComponent>(e)) {
                RenderComponent rc = *s.getRegistry()->getComponent<RenderComponent>(e);
                json renderjson;
                renderjson["mesh"] = rc.mesh;
                renderjson["material"] = rc.material;
                renderjson["shader"] = rc.shader;
                entity["renderComponent"] = renderjson;
            }
            if (s.getRegistry()->entityHasComponent<TransformComponent>(e)) {
                TransformComponent tc = *s.getRegistry()->getComponent<TransformComponent>(e);
                json transformjson;
                transformjson["position"] = dumpVec3(tc.position);
                transformjson["rotation"] = dumpVec3(tc.rotation);
                transformjson["scale"] = dumpVec3(tc.scale);
                entity["transformComponent"] = transformjson;
            }
            if (s.getRegistry()->entityHasComponent<LightComponent>(e)) {
                LightComponent lc = *s.getRegistry()->getComponent<LightComponent>(e);
                json lightjson;
                lightjson["color"] = dumpVec3(lc.color);
                lightjson["type"] = lc.type;
                entity["lightComponent"] = lightjson;
            }
            entities.push_back(entity);
        }
        j["entities"] = entities;
        j["skybox"] = s.getSkybox()->getTexture();
        outstream << j.dump(4);
        outstream.close();
    }
}

void Project::loadFromFile() {
    assetBank.fillWithDefaults();
    std::ifstream infile = std::ifstream(baseDirectory + "/" + name + "/" + name + ".ice");
    json j;
    infile >> j;
    infile.close();

    std::vector<std::string> sceneNames = j["scenes"];
    json meshesNames = j["meshes"];
    json materialNames = j["materials"];
    json shaderNames = j["shaders"];
    json textureNames = j["textures2D"];
    json cubeMapNames = j["cubeMaps"];

    cameraPosition = JsonParser::parseVec3(j["camera_position"]);
    cameraRotation = JsonParser::parseVec3(j["camera_rotation"]);

    std::string path = baseDirectory + "/" + name + "/Assets/";

    loadAssetsOfType<Texture2D>(path, textureNames);
    loadAssetsOfType<TextureCube>(path, cubeMapNames);
    loadAssetsOfType<Mesh>(path, meshesNames);
    loadAssetsOfType<Material>(path, materialNames);
    loadAssetsOfType<Shader>(path, shaderNames);

    for (auto s : sceneNames) {
        infile = std::ifstream(baseDirectory + "/" + name + "/Scenes/" + s + ".ics");
        json scenejson;
        infile >> scenejson;
        infile.close();

        Scene scene = Scene(scenejson["name"], new Registry());

        if (scenejson["skybox"] != "null") {
            scene.setSkybox(Skybox((AssetUID) scenejson["skybox"]));
        }
        for (json jentity : scenejson["entities"]) {
            //Entity e = scene.addEntity();
            if (!jentity["transformComponent"].is_null()) {
                json tj = jentity["transformComponent"];
                TransformComponent tc = {.position = JsonParser::parseVec3(tj["position"]),
                                         .rotation = JsonParser::parseVec3(tj["rotation"]),
                                         .scale = JsonParser::parseVec3(tj["scale"])};
                //scene.addComponent(tc);
            }
            if (!jentity["renderComponent"].is_null()) {
                json rj = jentity["renderComponent"];
                //RenderComponent* rc = new RenderComponent((AssetUID)rj["mesh"], (AssetUID)rj["material"], (AssetUID)rj["shader"]);
                //e->addComponent(rc);
            }
            if (!jentity["lightComponent"].is_null()) {
                json lj = jentity["lightComponent"];
                //LightComponent* lc = new LightComponent(PointLight, JsonParser::parseVec3(lj["color"]));
                //e->addComponent(lc);
            }
            //scene.addEntity(jentity["name"], e);
        }
        for (json jentity : scenejson["entities"]) {
            //scene.setParent(jentity["name"], jentity["parent"]);
        }
        scenes.push_back(scene);
    }
}

void Project::writeMaterialFile(const std::string &mtlName, const Material &mtl) {
    std::ofstream infile = std::ofstream(baseDirectory + "/" + name + "/Assets/Materials/" + mtlName + ".icm");
    json j;
    j["type"] = "phong";
    j["albedo"] = dumpVec3(mtl.getAlbedo());
    j["specular"] = dumpVec3(mtl.getSpecular());
    j["ambient"] = dumpVec3(mtl.getAmbient());
    j["alpha"] = mtl.getAlpha();
    j["diffuseMap"] = mtl.getDiffuseMap();
    j["specularMap"] = mtl.getSpecularMap();
    j["ambientMap"] = mtl.getAmbientMap();
    j["normalMap"] = mtl.getNormalMap();
    infile << j.dump(4);
    infile.close();
}

void Project::copyAssetFile(const std::string &folder, const std::string &assetName, const std::string &src) {
    std::ifstream srcStream(src, std::ios::binary);
    std::string dst = baseDirectory + "/" + name + "/Assets/" + folder + "/" + assetName + src.substr(src.find_last_of("."));
    std::ofstream dstStream(dst, std::ios::binary);

    dstStream << srcStream.rdbuf();
    dstStream.flush();
    srcStream.close();
    dstStream.close();
}

bool Project::renameAsset(const AssetPath &oldName, const AssetPath &newName) {
    if (newName.getName() == "" || newName.prefix() != oldName.prefix()) {
        return false;
    }
    if (assetBank.renameAsset(oldName, newName)) {
        std::string path = baseDirectory + "/" + name + "/Assets/";
        for (auto file : getFilesInDir(path + oldName.prefix())) {
            if (file.substr(0, file.find_last_of(".")) == oldName.getName()) {
                if (rename((path + oldName.prefix() + file).c_str(), (path + oldName.prefix() + (newName.getName() + file.substr(file.find_last_of(".")))).c_str()) == 0) {
                    return true;
                }
            }
        }
        return true;
    }
    return false;
}

std::vector<std::string> Project::getFilesInDir(const std::string &folder) {
    std::vector<std::string> files;
    for (const auto &entry : fs::directory_iterator(folder)) {
        std::string sp = entry.path().string();
        files.push_back(sp.substr(sp.find_last_of("/") + 1));
    }
    return files;
}

std::vector<Scene> &Project::getScenes() {
    return scenes;
}

void Project::setScenes(const std::vector<Scene> &scenes) {
    Project::scenes = scenes;
}

AssetBank *Project::getAssetBank() {
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

const Eigen::Vector3f &Project::getCameraPosition() const {
    return cameraPosition;
}

const Eigen::Vector3f &Project::getCameraRotation() const {
    return cameraRotation;
}
}  // namespace ICE