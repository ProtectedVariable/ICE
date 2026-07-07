#include "DefaultLoaders.h"

#include <memory>

#include "AssetBank.h"
#include "MaterialLoader.h"
#include "MeshLoader.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"

namespace ICE {
void registerDefaultLoaders(AssetBank &bank) {
    bank.addLoader<Texture2D>(std::make_shared<Texture2DLoader>());
    bank.addLoader<TextureCube>(std::make_shared<TextureCubeLoader>());
    bank.addLoader<Model>(std::make_shared<ModelLoader>(bank));
    bank.addLoader<Shader>(std::make_shared<ShaderLoader>());
    bank.addLoader<Material>(std::make_shared<MaterialLoader>());
    bank.addLoader<Mesh>(std::make_shared<MeshLoader>());
}
}  // namespace ICE
