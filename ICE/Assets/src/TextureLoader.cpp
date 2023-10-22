//
// Created by Thomas Ibanez on 29.07.21.
//

#include "TextureLoader.h"

#include <Texture.h>

namespace ICE {
std::shared_ptr<Texture2D> Texture2DLoader::load(const std::vector<std::filesystem::path> &file) {
    auto texture = graphics_factory->createTexture2D(file[0].string());
    texture->setSources(file);
    return texture;
}

std::shared_ptr<TextureCube> TextureCubeLoader::load(const std::vector<std::filesystem::path> &file) {
    auto texture = graphics_factory->createTextureCube(file[0].string());
    texture->setSources(file);
    return texture;
}
}  // namespace ICE