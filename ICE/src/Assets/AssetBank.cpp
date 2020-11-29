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
        meshes["__ice__cube__"] = OBJLoader::loadFromOBJ("Assets/bunny.obj");
        shaders["__ice__base_shader__"] = Shader::Create("Assets/test.vs", "Assets/test.fs");
        materials["__ice__base_material__"] = new Material(shaders["__ice__base_shader__"]);
    }
}