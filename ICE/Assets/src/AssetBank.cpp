//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"

#include <ICEException.h>
#include <Model.h>

#include "MaterialLoader.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"

namespace ICE {

AssetBank::AssetBank(const std::shared_ptr<GraphicsFactory> &factory) : graphics_factory(factory) {
    loader.AddLoader<Texture2D>(std::make_shared<Texture2DLoader>(factory));
    loader.AddLoader<TextureCube>(std::make_shared<TextureCubeLoader>(factory));
    loader.AddLoader<Model>(std::make_shared<ModelLoader>(factory, *this));
    loader.AddLoader<Shader>(std::make_shared<ShaderLoader>(factory));
    loader.AddLoader<Material>(std::make_shared<MaterialLoader>());
}

bool AssetBank::nameInUse(const AssetPath &name) {
    return !(nameMapping.find(name) == nameMapping.end());
}
}  // namespace ICE