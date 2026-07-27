#include "DefaultLoaders.h"

#include <memory>

#include "AssetBank.h"
#include "AudioClipLoader.h"
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
    // The AudioClip loader lives in the `audio` module (its decoders are a private dependency
    // there); its "Audio" path prefix is pre-registered alongside the other built-ins.
    bank.addLoader<AudioClip>(std::make_shared<AudioClipLoader>());
}
}  // namespace ICE
