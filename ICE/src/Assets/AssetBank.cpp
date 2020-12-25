//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"
#include <Util/OBJLoader.h>
#include <Util/ICEException.h>

namespace ICE {

    AssetBank::AssetBank(): meshes(std::unordered_map<std::string, Mesh*>()), materials(std::unordered_map<std::string, Material*>()), shaders(std::unordered_map<std::string, Shader*>()) {
        meshes.insert({"__ice__cube", OBJLoader::loadFromOBJ("Assets/Meshes/cube.obj")});
        meshes.insert({"__ice__sphere", OBJLoader::loadFromOBJ("Assets/Meshes/sphere.obj")});
        shaders["__ice__phong_shader"] = Shader::Create("Assets/Shaders/phong.vs", "Assets/Shaders/phong.fs");
        shaders["__ice__normal_shader"] = Shader::Create("Assets/Shaders/normal.vs", "Assets/Shaders/normal.fs");
        shaders["__ice__picking_shader"] = Shader::Create("Assets/Shaders/picking.vs", "Assets/Shaders/picking.fs");
        materials.insert({"__ice__base_material", new Material(Eigen::Vector3f(0.8f,0.8f,0.8f), Eigen::Vector3f(1,1,1), Eigen::Vector3f(1,1,1), 16.0f)});
    }

    Mesh* AssetBank::getMesh(const std::string &name) {
        return meshes.find(name)->second;
    }

    Material* AssetBank::getMaterial(const std::string &name) {
        return materials.find(name)->second;
    }

    Shader* AssetBank::getShader(const std::string &name) {
        return shaders[name];
    }

    Texture* AssetBank::getTexture(const std::string &name) {
        return textures[name];
    }

    const std::unordered_map<std::string, Mesh*> &AssetBank::getMeshes() const {
        return meshes;
    }

    const std::unordered_map<std::string, Material*> &AssetBank::getMaterials() const {
        return materials;
    }

    const std::unordered_map<std::string, Shader *> &AssetBank::getShaders() const {
        return shaders;
    }

    const std::unordered_map<std::string, Texture *> &AssetBank::getTextures() const {
        return textures;
    }

    bool AssetBank::addMesh(const std::string &name, Mesh* mesh) {
        if(meshes.find(name) == meshes.end()) {
            meshes.insert({name, mesh});
            return true;
        }
        return false;
    }

    bool AssetBank::addMaterial(const std::string &name, Material* mtl) {
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

    bool AssetBank::addTexture(const std::string &name, Texture *texture) {
        if(textures.find(name) == textures.end()) {
            textures[name] = texture;
            return true;
        }
        return false;
    }

    bool AssetBank::renameAsset(const std::string &oldName, const std::string &newName) {
        if(oldName.find(ICE_ASSET_PREFIX) != std::string::npos || newName.find(ICE_ASSET_PREFIX) != std::string::npos) {
            return false;
        }
        if(meshes.find(oldName) != meshes.end() && meshes.find(newName) == meshes.end()) {
            Mesh* m = meshes.at(oldName);
            meshes.insert({newName, m});
            meshes.erase(oldName);
            return true;
        } else if(materials.find(oldName) != materials.end() && materials.find(newName) == materials.end()) {
            Material* m = materials.at(oldName);
            materials.insert({newName, m});
            materials.erase(oldName);
            return true;
        } else if(shaders.find(oldName) != shaders.end() && shaders.find(newName) == shaders.end()) {
            shaders[newName] = shaders[oldName];
            shaders.erase(oldName);
            return true;
        } else if(textures.find(oldName) != textures.end() && textures.find(newName) == textures.end()) {
            textures[newName] = textures[oldName];
            textures.erase(oldName);
            return true;
        }
        return false;
    }

    std::string AssetBank::getName(const void *ptr) {
        for(const auto& e : meshes) {
            if(e.second == ptr) {
                return e.first;
            }
        }
        for(const auto& e : materials) {
            if(e.second == ptr) {
                return e.first;
            }
        }
        for(const auto& e : shaders) {
            if(e.second == ptr) {
                return e.first;
            }
        }
        for(const auto& e : textures) {
            if(e.second == ptr) {
                return e.first;
            }
        }
        throw ICEException();
    }
}