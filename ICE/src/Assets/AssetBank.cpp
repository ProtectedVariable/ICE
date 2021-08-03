//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"
#include "TextureLoader.h"
#include "MeshLoader.h"
#include "ShaderLoader.h"
#include "MaterialLoader.h"
#include <Util/OBJLoader.h>
#include <Util/ICEException.h>

namespace ICE {

    AssetBank::AssetBank(): resources(std::unordered_map<AssetUID, Resource*>()), nameMapping(std::unordered_map<AssetPath, AssetUID>()), loader(ResourceLoader()) {
        loader.AddLoader<Texture2D>(new Texture2DLoader());
        loader.AddLoader<TextureCube>(new TextureCubeLoader());
        loader.AddLoader<Mesh>(new MeshLoader());
        loader.AddLoader<Shader>(new ShaderLoader());
        loader.AddLoader<Material>(new MaterialLoader());
    }

    void AssetBank::fillWithDefaults() {
        addResource<Mesh>("__ice__cube", {"Assets/Meshes/cube.obj"});
        addResource<Mesh>("__ice__sphere", {"Assets/Meshes/sphere.obj"});
        addResource<Shader>("__ice__phong_shader", {"Assets/Shaders/phong.vs", "Assets/Shaders/phong.fs"});
        addResource<Shader>("__ice__normal_shader", {"Assets/Shaders/normal.vs", "Assets/Shaders/normal.fs"});
        addResource<Shader>("__ice__picking_shader", {"Assets/Shaders/picking.vs", "Assets/Shaders/picking.fs"});
        addResource<TextureCube>("__ice__skybox", {"Assets/Textures/skybox.png"});
        addResource<Material>("__ice__base_material", new Resource(new Material(Eigen::Vector3f(0.8f,0.8f,0.8f), Eigen::Vector3f(1,1,1), Eigen::Vector3f(1,1,1), 16.0f), {}));

    }

    bool AssetBank::nameInUse(const std::string &name) {
        if(name == "") return true;
        return !(nameMapping.find(name) == nameMapping.end());
    }
}