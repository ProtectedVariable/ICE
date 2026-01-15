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

AssetBank::AssetBank() {
    loader.AddLoader<Texture2D>(std::make_shared<Texture2DLoader>());
    loader.AddLoader<TextureCube>(std::make_shared<TextureCubeLoader>());
    loader.AddLoader<Model>(std::make_shared<ModelLoader>(*this));
    loader.AddLoader<Shader>(std::make_shared<ShaderLoader>());
    loader.AddLoader<Material>(std::make_shared<MaterialLoader>());
}

bool AssetBank::nameInUse(const AssetPath &name) {
    return !(nameMapping.find(name) == nameMapping.end());
}
}  // namespace ICE