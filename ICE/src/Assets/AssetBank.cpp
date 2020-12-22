//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"
#include <Util/OBJLoader.h>
namespace ICE {

    Mesh* AssetBank::getMesh(const std::string &name) {
        return &meshes.at(name);
    }

    Material* AssetBank::getMaterial(const std::string &name) {
        return &materials.at(name);
    }

    AssetBank::AssetBank(): meshes(std::unordered_map<std::string, Mesh>()), materials(std::unordered_map<std::string, Material>()), shaders(std::unordered_map<std::string, Shader*>()) {
        meshes.insert({"__ice__cube", OBJLoader::loadFromOBJ("Assets/Meshes/cube.obj")});
        meshes.insert({"__ice__sphere", OBJLoader::loadFromOBJ("Assets/Meshes/sphere.obj")});
        shaders["__ice__phong_shader"] = Shader::Create("Assets/Shaders/phong.vs", "Assets/Shaders/phong.fs");
        shaders["__ice__normal_shader"] = Shader::Create("Assets/Shaders/normal.vs", "Assets/Shaders/normal.fs");
        shaders["__ice__picking_shader"] = Shader::Create("Assets/Shaders/picking.vs", "Assets/Shaders/picking.fs");
        materials.insert({"__ice__base_material", Material(shaders["__ice__phong_shader"], Eigen::Vector3f(0.8f,0.8f,0.8f), Eigen::Vector3f(1,1,1), Eigen::Vector3f(1,1,1), 16.0f)});
    }

    Shader *AssetBank::getShader(const std::string &name) {
        return shaders[name];
    }

    const std::unordered_map<std::string, Mesh> &AssetBank::getMeshes() const {
        return meshes;
    }

    const std::unordered_map<std::string, Material> &AssetBank::getMaterials() const {
        return materials;
    }

    const std::unordered_map<std::string, Shader *> &AssetBank::getShaders() const {
        return shaders;
    }

    bool AssetBank::addMesh(const std::string &name, const Mesh& mesh) {
        if(meshes.find(name) == meshes.end()) {
            meshes.insert({name, mesh});
            return true;
        }
        return false;
    }

    bool AssetBank::addMaterial(const std::string &name, const Material& mtl) {
        if(materials.find(name) == materials.end()) {
            materials.insert({name, mtl});
            return true;
        }
        return false;
    }

    bool AssetBank::addShader(const std::string &name, Shader* shader) {
        if(shaders.find(name) == shaders.end()) {
            shaders[name] = shader;
            return true;
        }
        return false;
    }

    bool AssetBank::renameAsset(const std::string &oldName, const std::string &newName) {
        if(oldName.find(ICE_ASSET_PREFIX) != std::string::npos || newName.find(ICE_ASSET_PREFIX) != std::string::npos) {
            return false;
        }
        if(meshes.find(oldName) != meshes.end() && meshes.find(newName) == meshes.end()) {
            Mesh m = meshes.at(oldName);
            meshes.insert({newName, m});
            meshes.erase(oldName);
            return true;
        } else if(materials.find(oldName) != materials.end() && materials.find(newName) == materials.end()) {
            Material m = materials.at(oldName);
            materials.insert({newName, m});
            materials.erase(oldName);
            return true;
        } else if(shaders.find(oldName) != shaders.end() && shaders.find(newName) == shaders.end()) {
            shaders[newName] = shaders[oldName];
            shaders.erase(oldName);
            return true;
        }
        return false;
    }
}