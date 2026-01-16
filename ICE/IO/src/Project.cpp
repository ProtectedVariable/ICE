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

#include "MaterialExporter.h"
#include "ShaderExporter.h"

namespace ICE {
Project::Project(const fs::path &base_directory, const std::string &m_name)
    : m_base_directory(base_directory / m_name),
      m_name(m_name),
      m_asset_bank(std::make_shared<AssetBank>()),
      m_gpu_registry(std::make_shared<GPURegistry>(std::make_shared<OpenGLFactory>(), m_asset_bank)) {
    cameraPosition.setZero();
    cameraRotation.setZero();
    constexpr std::string_view assets_folder = "Assets";
    m_materials_directory = m_base_directory / assets_folder / "Materials";
    m_shaders_directory = m_base_directory / assets_folder / "Shaders";
    m_textures_directory = m_base_directory / assets_folder / "Textures";
    m_cubemaps_directory = m_base_directory / assets_folder / "Cubemaps";
    m_models_directory = m_base_directory / assets_folder / "Models";
    m_meshes_directory = m_base_directory / assets_folder / "Meshes";
    m_scenes_directory = m_base_directory / "Scenes";
}

bool Project::CreateDirectories() {
    fs::create_directories(m_scenes_directory);
    try {
        fs::copy("Assets", m_base_directory / "Assets", fs::copy_options::recursive);
    } catch (std::filesystem::filesystem_error &e) {
        Logger::Log(Logger::FATAL, "IO", "Could not copy default assets: %s", e.what());
    }
    m_asset_bank->addAsset<Shader>("solid", {m_shaders_directory / "solid.shader.json"});
    m_asset_bank->addAsset<Shader>("phong", {m_shaders_directory / "phong.shader.json"});
    m_asset_bank->addAsset<Shader>("normal", {m_shaders_directory / "normal.shader.json"});
    m_asset_bank->addAsset<Shader>("pbr", {m_shaders_directory / "pbr.shader.json"});
    m_asset_bank->addAsset<Shader>("lastpass", {m_shaders_directory / "lastpass.shader.json"});
    m_asset_bank->addAsset<Shader>("__ice__picking_shader", {m_shaders_directory / "picking.shader.json"});

    m_asset_bank->addAsset<Material>("base_mat", {m_materials_directory / "base_mat.material.json"});

    m_asset_bank->addAsset<Mesh>("cube", {m_meshes_directory / "cube.obj"});
    m_asset_bank->addAsset<Mesh>("sphere", {m_meshes_directory / "sphere.obj"});

    m_asset_bank->addAsset<Texture2D>("Editor/folder", {m_textures_directory / "Editor" / "folder.png"});
    m_asset_bank->addAsset<Texture2D>("Editor/shader", {m_textures_directory / "Editor" / "shader.png"});

    m_scenes.push_back(std::make_shared<Scene>("MainScene"));
    setCurrentScene(getScenes()[0]);
    return true;
}

fs::path Project::getBaseDirectory() const {
    return m_base_directory;
}

std::string Project::getName() const {
    return m_name;
}

void Project::writeToFile(const std::shared_ptr<Camera> &editorCamera) {
    std::ofstream outstream;
    outstream.open(m_base_directory / (m_name + ".ice"));
    json j;

    j["camera_position"] = dumpVec3(editorCamera->getPosition());
    j["camera_rotation"] = dumpVec3(editorCamera->getRotation());

    std::vector<json> vec;
    for (const auto &s : m_scenes) {
        vec.push_back(s->getName());
    }
    j["scenes"] = vec;
    vec.clear();

    for (const auto &[asset_id, mesh] : m_asset_bank->getAll<Model>()) {
        vec.push_back(dumpAsset(asset_id, mesh));
    }
    j["models"] = vec;
    vec.clear();

    for (const auto &[asset_id, mesh] : m_asset_bank->getAll<Mesh>()) {
        vec.push_back(dumpAsset(asset_id, mesh));
    }
    j["meshes"] = vec;
    vec.clear();

    for (const auto &[asset_id, material] : m_asset_bank->getAll<Material>()) {
        auto mtlName = m_asset_bank->getName(asset_id).getName();

        fs::path path = m_materials_directory.parent_path() / (m_asset_bank->getName(asset_id).prefix() + mtlName + ".material.json");
        fs::create_directories(path.parent_path());
        MaterialExporter().writeToJson(path, *material);

        material->setSources({path});

        vec.push_back(dumpAsset(asset_id, material));
    }
    j["materials"] = vec;
    vec.clear();

    for (const auto &[asset_id, shader] : m_asset_bank->getAll<Shader>()) {
        auto mtlName = m_asset_bank->getName(asset_id).getName();

        fs::path path = m_shaders_directory.parent_path() / (m_asset_bank->getName(asset_id).prefix() + mtlName + ".shader.json");
        fs::create_directories(path.parent_path());
        ShaderExporter().writeToJson(path, *shader);

        shader->setSources({path});

        vec.push_back(dumpAsset(asset_id, shader));
    }
    j["shaders"] = vec;
    vec.clear();

    for (const auto &[asset_id, texture] : m_asset_bank->getAll<Texture2D>()) {
        vec.push_back(dumpAsset(asset_id, texture));
    }
    j["textures2D"] = vec;
    vec.clear();

    for (const auto &[asset_id, texture] : m_asset_bank->getAll<TextureCube>()) {
        vec.push_back(dumpAsset(asset_id, texture));
    }
    j["cubeMaps"] = vec;

    outstream << j.dump(4);
    outstream.close();

    for (const auto &s : m_scenes) {
        outstream.open(m_scenes_directory / (s->getName() + ".ics"));
        j.clear();

        j["m_name"] = s->getName();
        json entities = json::array();
        for (auto e : s->getRegistry()->getEntities()) {
            json entity;
            entity["id"] = e;
            entity["m_name"] = s->getAlias(e);
            entity["parent"] = s->getGraph()->getParentID(e);

            if (s->getRegistry()->entityHasComponent<RenderComponent>(e)) {
                RenderComponent rc = *s->getRegistry()->getComponent<RenderComponent>(e);
                json renderjson;
                renderjson["mesh"] = rc.mesh;
                renderjson["material"] = rc.material;
                entity["renderComponent"] = renderjson;
            }
            if (s->getRegistry()->entityHasComponent<TransformComponent>(e)) {
                TransformComponent tc = *s->getRegistry()->getComponent<TransformComponent>(e);
                json transformjson;
                transformjson["position"] = dumpVec3(tc.getPosition());
                transformjson["rotation"] = dumpVec3(tc.getRotationEulerDeg());
                transformjson["scale"] = dumpVec3(tc.getScale());
                entity["transformComponent"] = transformjson;
            }
            if (s->getRegistry()->entityHasComponent<LightComponent>(e)) {
                LightComponent lc = *s->getRegistry()->getComponent<LightComponent>(e);
                json lightjson;
                lightjson["color"] = dumpVec3(lc.color);
                lightjson["type"] = lc.type;
                entity["lightComponent"] = lightjson;
            }
            entities.push_back(entity);
        }
        j["entities"] = entities;
        outstream << j.dump(4);
        outstream.close();
    }
}

json Project::dumpAsset(AssetUID uid, const std::shared_ptr<Asset> &asset) {
    auto asset_path = m_asset_bank->getName(uid);
    json tmp;
    auto paths = asset->getSources();
    std::vector<std::string> sources(paths.size());
    std::transform(paths.begin(), paths.end(), sources.begin(), [this](const fs::path &path) {
        auto relative = path.lexically_relative(m_base_directory);
        return relative.string();
    });
    tmp["bank_path"] = asset_path.toString();
    tmp["uid"] = uid;
    tmp["sources"] = sources;
    return tmp;
}

void Project::loadFromFile() {
    std::ifstream infile = std::ifstream(m_base_directory / (m_name + ".ice"));
    json j;
    infile >> j;
    infile.close();

    std::vector<std::string> sceneNames = j["scenes"];
    json models = j["models"];
    json meshes = j["meshes"];
    json material = j["materials"];
    json shader = j["shaders"];
    json texture = j["textures2D"];
    json cubeMap = j["cubeMaps"];

    cameraPosition = JsonParser::parseVec3(j["camera_position"]);
    cameraRotation = JsonParser::parseVec3(j["camera_rotation"]);

    loadAssetsOfType<Shader>(shader);
    loadAssetsOfType<Texture2D>(texture);
    loadAssetsOfType<TextureCube>(cubeMap);
    loadAssetsOfType<Material>(material);
    loadAssetsOfType<Mesh>(meshes);
    loadAssetsOfType<Model>(models);

    for (const auto &s : sceneNames) {
        infile = std::ifstream(m_scenes_directory / (s + ".ics"));
        json scenejson;
        infile >> scenejson;
        infile.close();

        Scene scene = Scene(scenejson["m_name"]);

        for (json jentity : scenejson["entities"]) {
            Entity e = jentity["id"];
            Entity parent = jentity["parent"];
            std::string alias = jentity["m_name"];

            scene.addEntity(e, alias, 0);

            if (!jentity["transformComponent"].is_null()) {
                json tj = jentity["transformComponent"];
                TransformComponent tc(JsonParser::parseVec3(tj["position"]), JsonParser::parseVec3(tj["rotation"]),
                                      JsonParser::parseVec3(tj["scale"]));
                scene.getRegistry()->addComponent(e, tc);
            }
            if (!jentity["renderComponent"].is_null()) {
                json rj = jentity["renderComponent"];
                RenderComponent rc(rj["mesh"], rj["material"]);
                scene.getRegistry()->addComponent(e, rc);
            }
            if (!jentity["lightComponent"].is_null()) {
                json lj = jentity["lightComponent"];
                LightComponent lc(static_cast<LightType>((int) lj["type"]), JsonParser::parseVec3(lj["color"]));
                scene.getRegistry()->addComponent(e, lc);
            }
        }
        for (json jentity : scenejson["entities"]) {
            Entity e = jentity["id"];
            Entity parent = jentity["parent"];
            scene.getGraph()->setParent(e, parent);
        }
        addScene(scene);
        //TODO: it would be better to save the current scene index
        setCurrentScene(getScenes()[0]);
    }
}

void Project::copyAssetFile(const fs::path &folder, const std::string &assetName, const fs::path &src) {
    auto subfolder = m_base_directory / "Assets" / folder;
    fs::create_directories(subfolder);

    auto dst = subfolder / (assetName + src.extension().string());
    std::ifstream srcStream(src, std::ios::binary);
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
    if (m_asset_bank->renameAsset(oldName, newName)) {
        auto path = m_base_directory / "Assets";
        for (const auto &file : getFilesInDir(path / oldName.prefix())) {
            if (file.substr(0, file.find_last_of(".")) == oldName.getName()) {
                if (rename((path / oldName.prefix() / file).string().c_str(),
                           (path / oldName.prefix() / (newName.getName() + file.substr(file.find_last_of(".")))).string().c_str())
                    == 0) {
                    return true;
                }
            }
        }
        return true;
    }
    return false;
}

std::vector<std::string> Project::getFilesInDir(const fs::path &folder) {
    std::vector<std::string> files;
    for (const auto &entry : fs::directory_iterator(folder)) {
        std::string sp = entry.path().string();
        files.push_back(sp.substr(sp.find_last_of("/") + 1));
    }
    return files;
}

std::vector<std::shared_ptr<Scene>> Project::getScenes() {
    return m_scenes;
}

void Project::setScenes(const std::vector<std::shared_ptr<Scene>> &scenes) {
    m_scenes = scenes;
}

std::shared_ptr<GPURegistry> Project::getGPURegistry() {
    return m_gpu_registry;
}

std::shared_ptr<AssetBank> Project::getAssetBank() {
    return m_asset_bank;
}

void Project::setAssetBank(const std::shared_ptr<AssetBank> &asset_bank) {
    m_asset_bank = asset_bank;
}

void Project::addScene(const Scene &scene) {
    m_scenes.push_back(std::make_shared<Scene>(scene));
}

void Project::setCurrentScene(const std::shared_ptr<Scene> &scene) {
    m_current_scene = scene;
}
std::shared_ptr<Scene> Project::getCurrentScene() const {
    return m_current_scene;
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