#pragma once

namespace ICE {
class AssetBank;

// Registers the engine's built-in asset loaders (mesh, model, material, shader,
// textures) onto the given bank. Lives in the `io` module because that is where
// the concrete loaders (and their Assimp/JSON dependencies) live; keeping this
// out of AssetBank's constructor is what breaks the assets <-> io dependency cycle.
void registerDefaultLoaders(AssetBank &bank);
}  // namespace ICE
