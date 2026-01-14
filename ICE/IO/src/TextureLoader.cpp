//
// Created by Thomas Ibanez on 29.07.21.
//

#include "TextureLoader.h"

#include <Logger.h>
#include <Texture.h>

namespace ICE {
std::shared_ptr<Texture2D> Texture2DLoader::load(const std::vector<std::filesystem::path> &file) {
    Logger::Log(Logger::VERBOSE, "IO", "Loading texture...");
    if (file.empty()) {
        return nullptr;
    }
    auto texture = std::make_shared<Texture2D>();
    texture->setSources(file);
    return texture;
}

std::shared_ptr<TextureCube> TextureCubeLoader::load(const std::vector<std::filesystem::path> &file) {
    Logger::Log(Logger::VERBOSE, "IO", "Loading cubemap...");
    auto texture = std::make_shared<TextureCube>();
    texture->setSources(file);
    return texture;
}
}  // namespace ICE