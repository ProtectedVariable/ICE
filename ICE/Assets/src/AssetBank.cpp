//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"

#include <ICEException.h>
#include <OBJLoader.h>

#include "MaterialLoader.h"
#include "MeshLoader.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"

namespace ICE {

AssetBank::AssetBank(const std::shared_ptr<GraphicsFactory> &factory) : graphics_factory(factory) {
    loader.AddLoader<Texture2D>(std::make_shared<Texture2DLoader>(factory));
    loader.AddLoader<TextureCube>(std::make_shared<TextureCubeLoader>(factory));
    loader.AddLoader<Mesh>(std::make_shared<MeshLoader>(factory));
    loader.AddLoader<Shader>(std::make_shared<ShaderLoader>(factory));
    loader.AddLoader<Material>(std::make_shared<MaterialLoader>());
}

void AssetBank::fillWithDefaults() {
    addAsset<Mesh>("__ice__cube", {"Assets/Meshes/cube.obj"});
    addAsset<Mesh>("__ice__sphere", {"Assets/Meshes/sphere.obj"});
    addAsset<Shader>("__ice__phong_shader", {"Assets/Shaders/phong.vs", "Assets/Shaders/phong.fs"});
    addAsset<Shader>("__ice__normal_shader", {"Assets/Shaders/normal.vs", "Assets/Shaders/normal.fs"});
    addAsset<Shader>("__ice__picking_shader", {"Assets/Shaders/picking.vs", "Assets/Shaders/picking.fs"});
    addAsset<TextureCube>("__ice__skybox", {"Assets/Textures/skybox.png"});
    addAsset<Material>("__ice__base_material", std::make_shared<Material>(Eigen::Vector3f(0.8f, 0.8f, 0.8f), Eigen::Vector3f(1, 1, 1), Eigen::Vector3f(1, 1, 1), 16.0f));
}

bool AssetBank::nameInUse(const AssetPath &name) {
    return !(nameMapping.find(name) == nameMapping.end());
}
}  // namespace ICE