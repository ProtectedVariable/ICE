//
// Created by Thomas Ibanez on 29.07.21.
//

#pragma once

#include <string>

#include "Asset.h"
#include "IAssetLoader.h"
#include "Texture.h"

namespace ICE {
class Texture2DLoader : public IAssetLoader<Texture2D> {
   public:
    Texture2DLoader() = default;
    std::shared_ptr<Texture2D> load(const std::vector<std::filesystem::path> &file) override;
};
class TextureCubeLoader : public IAssetLoader<TextureCube> {
   public:
    TextureCubeLoader() = default;
    std::shared_ptr<TextureCube> load(const std::vector<std::filesystem::path> &file) override;
};
}  // namespace ICE
