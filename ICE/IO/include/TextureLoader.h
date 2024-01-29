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
    Texture2DLoader(const std::shared_ptr<GraphicsFactory> &factory) : IAssetLoader<Texture2D>(factory) {}
    std::shared_ptr<Texture2D> load(const std::vector<std::filesystem::path> &file) override;
};
class TextureCubeLoader : public IAssetLoader<TextureCube> {
   public:
    TextureCubeLoader(const std::shared_ptr<GraphicsFactory> &factory) : IAssetLoader<TextureCube>(factory) {}
    std::shared_ptr<TextureCube> load(const std::vector<std::filesystem::path> &file) override;
};
}  // namespace ICE
