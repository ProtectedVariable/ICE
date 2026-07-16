//
// Created by Thomas Ibanez on 09.12.20.
//

#include "Project.h"

#include <Entity.h>
#include <JsonParser.h>
#include <LightComponent.h>
#include <Model.h>
#include <OpenGLFactory.h>
#include <RenderComponent.h>
#include <Scene.h>
#include <SkeletonPoseComponent.h>
#include <TransformComponent.h>

#include <fstream>
#include <iostream>
#include <optional>
#include <typeindex>
#include <unordered_set>

#include "DefaultLoaders.h"
#include "MaterialExporter.h"
#include "ModelLoader.h"
#include "ShaderExporter.h"
#include <SkinningComponent.h>

namespace ICE {
namespace {
// The six built-in asset kinds are persisted in their own named sections; everything else (plugin
// types) goes through the generic "assets" section. Keep these in sync with the built-in prefixes
// pre-registered in AssetPath.
bool isBuiltinAssetPrefix(const std::string &prefix) {
    static const std::unordered_set<std::string> builtins = {"Textures", "CubeMaps", "Meshes", "Models", "Materials", "Shaders"};
    return builtins.find(prefix) != builtins.end();
}
}  // namespace
Project::Project(const fs::path &base_directory, const std::string &m_name)
    : m_base_directory(base_directory / m_name),
      m_name(m_name),
      m_asset_bank(std::make_shared<AssetBank>()),
      m_gpu_registry(std::make_shared<GPURegistry>(std::make_shared<OpenGLFactory>(), m_asset_bank)) {
    registerDefaultLoaders(*m_asset_bank);
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

    addScene(Scene("MainScene"));  // addScene wires the asset bank into the scene
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
    vec.clear();

    // Generic section for plugin-defined asset kinds (anything whose path prefix is not one of the
    // six built-ins). Keyed by prefix so load can route each entry to the right erased loader. Any
    // entries whose plugin was missing at load are re-emitted verbatim first, so they are preserved.
    std::vector<json> custom_assets = m_unknown_assets;
    for (const auto &entry : m_asset_bank->getAllEntries()) {
        if (!entry.asset) {
            continue;  // reservation still loading / failed load: nothing to persist
        }
        const auto components = entry.path.getPath();
        std::string type_prefix = components.empty() ? "" : components.front();
        if (type_prefix.empty() || isBuiltinAssetPrefix(type_prefix)) {
            continue;  // built-ins are saved in their own sections above
        }
        AssetUID uid = m_asset_bank->getUID(entry.path);
        json dumped = dumpAsset(uid, entry.asset);
        dumped["prefix"] = type_prefix;
        custom_assets.push_back(dumped);
    }
    j["assets"] = custom_assets;

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
            if (s->getRegistry()->entityHasComponent<AnimationComponent>(e)) {
                AnimationComponent ac = *s->getRegistry()->getComponent<AnimationComponent>(e);
                json animjson;
                animjson["currentAnimation"] = ac.currentAnimation;
                animjson["currentTime"] = ac.currentTime;
                animjson["speed"] = ac.speed;
                animjson["playing"] = ac.playing;
                animjson["loop"] = ac.loop;
                entity["animationComponent"] = animjson;
            }
            if (s->getRegistry()->entityHasComponent<SkeletonPoseComponent>(e)) {
                SkeletonPoseComponent sc = *s->getRegistry()->getComponent<SkeletonPoseComponent>(e);
                json spjson;
                spjson["skeletonModel"] = sc.skeletonModel;
                spjson["bone_entity"] = sc.bone_entity;
                std::vector<json> bone_transforms;
                for (const auto &tr : sc.bone_transform) {
                    bone_transforms.push_back(JsonParser::dumpMat4(tr));
                }
                spjson["bone_transforms"] = bone_transforms;
                entity["skeletonPoseComponent"] = spjson;
            }
            if (s->getRegistry()->entityHasComponent<SkinningComponent>(e)) {
                SkinningComponent sc = *s->getRegistry()->getComponent<SkinningComponent>(e);
                json scjson;
                scjson["skeleton_entity"] = sc.skeleton_entity;
                entity["skinningComponent"] = scjson;
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
    if (!infile.is_open()) {
        Logger::Log(Logger::ERROR, "IO", "Could not open project file '%s'", (m_base_directory / (m_name + ".ice")).string().c_str());
        return;
    }
    json j;
    try {
        infile >> j;
    } catch (const std::exception &e) {
        Logger::Log(Logger::ERROR, "IO", "Failed to parse project file: %s", e.what());
        return;
    }
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

    // Generic section for plugin-defined asset kinds. Route each entry to the right loader via its
    // path prefix (AssetPath::typeForPrefix). If the type is unknown (its plugin isn't loaded) or has
    // no loader, warn and keep the raw entry so the next save preserves it instead of dropping it.
    m_unknown_assets.clear();
    if (j.contains("assets")) {
        for (const auto &asset : j["assets"]) {
            std::string prefix = asset.value("prefix", std::string());
            std::optional<std::type_index> type;
            if (!prefix.empty()) {
                type = AssetPath::typeForPrefix(prefix);
            }
            if (!type.has_value()) {
                Logger::Log(Logger::WARNING, "IO", "No registered asset type for prefix '%s'; preserving entry across save", prefix.c_str());
                m_unknown_assets.push_back(asset);
                continue;
            }
            AssetUID uid = asset["uid"];
            std::string bank_path = asset["bank_path"];
            std::vector<fs::path> sources;
            for (const auto &entry : asset["sources"]) {
                sources.push_back(m_base_directory / std::string(entry));
            }
            try {
                m_asset_bank->addAssetWithSpecificUID(type.value(), AssetPath(bank_path), sources, uid);
            } catch (const std::exception &e) {
                Logger::Log(Logger::WARNING, "IO", "Could not load custom asset '%s' (%s); preserving entry", bank_path.c_str(), e.what());
                m_unknown_assets.push_back(asset);
            }
        }
    }

    for (const auto &s : sceneNames) {
        infile = std::ifstream(m_scenes_directory / (s + ".ics"));
        if (!infile.is_open()) {
            Logger::Log(Logger::ERROR, "IO", "Could not open scene file '%s'", s.c_str());
            continue;
        }
        json scenejson;
        try {
            infile >> scenejson;
        } catch (const std::exception &e) {
            Logger::Log(Logger::ERROR, "IO", "Failed to parse scene '%s': %s", s.c_str(), e.what());
            continue;
        }
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
            if (!jentity["animationComponent"].is_null()) {
                json aj = jentity["animationComponent"];
                AnimationComponent ac;
                ac.currentAnimation = aj["currentAnimation"];
                ac.currentTime = aj["currentTime"];
                ac.speed = aj["speed"];
                ac.playing = aj["playing"];
                ac.loop = aj["loop"];
                scene.getRegistry()->addComponent(e, ac);
            }
            if (!jentity["skeletonPoseComponent"].is_null()) {
                json sj = jentity["skeletonPoseComponent"];
                SkeletonPoseComponent sc;
                sc.skeletonModel = sj["skeletonModel"];
                sc.bone_entity = sj["bone_entity"].get<std::unordered_map<std::string, Entity>>();
                std::vector<Eigen::Matrix4f> bone_transforms;
                for (const auto &jt : sj["bone_transforms"]) {
                    bone_transforms.push_back(JsonParser().readMat4(jt));
                }
                sc.bone_transform = bone_transforms;
                scene.getRegistry()->addComponent(e, sc);
            }
            if (!jentity["skinningComponent"].is_null()) {
                json skj = jentity["skinningComponent"];
                SkinningComponent sc;
                sc.skeleton_entity = skj["skeleton_entity"];
                scene.getRegistry()->addComponent(e, sc);
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
    if (!srcStream.is_open()) {
        Logger::Log(Logger::ERROR, "IO", "Could not open source asset '%s'", src.string().c_str());
        return;
    }
    std::ofstream dstStream(dst, std::ios::binary);
    if (!dstStream.is_open()) {
        Logger::Log(Logger::ERROR, "IO", "Could not open destination '%s' for asset copy", dst.string().c_str());
        return;
    }

    dstStream << srcStream.rdbuf();
    dstStream.flush();
    srcStream.close();
    dstStream.close();
}

AssetUID Project::importModel(const std::string &name, const fs::path &src) {
    // Copy the source file into <project>/Assets/Models (keeping its extension) and register it.
    copyAssetFile("Models", name, src);
    fs::path dst = m_models_directory / (name + src.extension().string());
    m_asset_bank->addAsset<Model>(name, {dst});
    return model(name);
}

AssetUID Project::requestModel(const std::string &name, const std::vector<fs::path> &sources) {
    // Resolve the one bank value the parse needs (the pbr shader UID) on the main thread, then stage
    // (pure) on a worker and commit (bank mutation) on the main thread in pump(). The ModelLoader is
    // captured by shared_ptr so it outlives the in-flight request; the commit runs only in pump(),
    // where the bank is alive.
    AssetUID pbr_shader_uid = m_asset_bank->getUID(AssetPath::WithTypePrefix<Shader>("pbr"));
    auto loader = std::make_shared<ModelLoader>(*m_asset_bank);
    AssetBank *bank = m_asset_bank.get();

    AssetBank::StageFn stage = [loader, sources, pbr_shader_uid]() -> std::shared_ptr<void> {
        return std::make_shared<StagedModel>(loader->stage(sources, pbr_shader_uid));
    };
    AssetBank::CommitFn commit = [loader, bank](const std::shared_ptr<void> &staged) -> std::shared_ptr<Asset> {
        auto staged_model = std::static_pointer_cast<StagedModel>(staged);
        return loader->commit(*staged_model, *bank);
    };
    return m_asset_bank->requestAsset<Model>(name, std::move(stage), std::move(commit));
}

AssetUID Project::mesh(const std::string &name) const {
    return m_asset_bank->getUID(AssetPath::WithTypePrefix<Mesh>(name));
}

AssetUID Project::material(const std::string &name) const {
    return m_asset_bank->getUID(AssetPath::WithTypePrefix<Material>(name));
}

AssetUID Project::model(const std::string &name) const {
    return m_asset_bank->getUID(AssetPath::WithTypePrefix<Model>(name));
}

bool Project::renameAsset(const AssetPath &oldName, const AssetPath &newName) {
    if (newName.getName() == "" || newName.prefix() != oldName.prefix()) {
        return false;
    }
    if (m_asset_bank->renameAsset(oldName, newName)) {
        auto path = m_base_directory / "Assets";
        for (const auto &file : getFilesInDir(path / oldName.prefix())) {
            // stem()/extension() handle files with no extension (the old substr(find_last_of("."))
            // threw std::out_of_range on those).
            fs::path fp(file);
            if (fp.stem().string() == oldName.getName()) {
                if (rename((path / oldName.prefix() / file).string().c_str(),
                           (path / oldName.prefix() / (newName.getName() + fp.extension().string())).string().c_str())
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
        // Use std::filesystem to extract the filename: splitting on '/' returned the whole
        // path on Windows, where the separator is '\'.
        files.push_back(entry.path().filename().string());
    }
    return files;
}

std::vector<std::shared_ptr<Scene>> Project::getScenes() {
    return m_scenes;
}

void Project::setScenes(const std::vector<std::shared_ptr<Scene>> &scenes) {
    m_scenes = scenes;
    // Keep spawn()/by-name lookups working on scenes set in bulk (e.g. after a load).
    for (auto &scene : m_scenes) {
        if (scene) {
            scene->setAssetBank(m_asset_bank);
        }
    }
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
    auto stored = std::make_shared<Scene>(scene);
    // Wire the project's asset bank into the scene so scene.spawn()/by-name lookups work without
    // the caller threading the bank through.
    stored->setAssetBank(m_asset_bank);
    m_scenes.push_back(stored);
}

void Project::setCurrentScene(const std::shared_ptr<Scene> &scene) {
    m_current_scene = scene;
}
std::shared_ptr<Scene> Project::getCurrentScene() const {
    return m_current_scene;
}

Scene &Project::createScene(const std::string &name) {
    addScene(Scene(name));
    auto scene = m_scenes.back();
    m_current_scene = scene;
    if (m_scene_activator) {
        m_scene_activator(scene);  // engine builds the scene's runtime systems
    }
    return *scene;
}

void Project::setSceneActivator(const std::function<void(const std::shared_ptr<Scene>&)> &activator) {
    m_scene_activator = activator;
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