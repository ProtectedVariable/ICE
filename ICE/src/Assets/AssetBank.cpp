//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"
#include <Util/OBJLoader.h>
namespace ICE {

    Mesh *AssetBank::getMesh(const std::string &name) {
        return meshes[name];
    }

    Material *AssetBank::getMaterial(const std::string &name) {
        return materials[name];
    }

    AssetBank::AssetBank(): meshes(std::unordered_map<std::string, Mesh*>()), materials(std::unordered_map<std::string, Material*>()), shaders(std::unordered_map<std::string, Shader*>()) {
        meshes["__ice__cube"] = OBJLoader::loadFromOBJ("Assets/Meshes/cube.obj");
        meshes["__ice__sphere"] = OBJLoader::loadFromOBJ("Assets/Meshes/sphere.obj");
        shaders["__ice__base_shader"] = Shader::Create("Assets/test.vs", "Assets/test.fs");
        shaders["__ice__picking_shader"] = Shader::Create("Assets/Shaders/picking.vs", "Assets/Shaders/picking.fs");
        materials["__ice__base_material"] = new Material(shaders["__ice__base_shader"]);
    }

    Shader *AssetBank::getShader(const std::string &name) {
        return shaders[name];
    }

    const std::unordered_map<std::string, Mesh *> &AssetBank::getMeshes() const {
        return meshes;
    }

    const std::unordered_map<std::string, Material *> &AssetBank::getMaterials() const {
        return materials;
    }

    const std::unordered_map<std::string, Shader *> &AssetBank::getShaders() const {
        return shaders;
    }
}