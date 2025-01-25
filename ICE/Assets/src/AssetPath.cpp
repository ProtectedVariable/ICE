//
// Created by Thomas Ibanez on 03.08.21.
//

#include "AssetPath.h"

#include <Material.h>
#include <Model.h>
#include <Shader.h>
#include <Texture.h>

namespace ICE {

std::unordered_map<std::type_index, std::string> AssetPath::typenames = {{typeid(Texture2D), "Textures"},
                                                                         {typeid(TextureCube), "CubeMaps"},
                                                                         {typeid(Model), "Models"},
                                                                         {typeid(Material), "Materials"},
                                                                         {typeid(Shader), "Shaders"}};

AssetPath::AssetPath(std::string path) {
    size_t last = 0;
    for (size_t i = 0; i < path.length(); i++) {
        if (path[i] == ASSET_PATH_SEPARATOR) {
            this->path.push_back(path.substr(last, i - last));
            last = i + 1;
        }
    }
    name = path.substr(last, path.length() - last);
}

std::string AssetPath::toString() const {
    return (prefix() + name);
}

std::vector<std::string> AssetPath::getPath() const {
    return path;
}

std::string AssetPath::getName() const {
    return name;
}

void AssetPath::setName(const std::string &name) {
    AssetPath::name = name;
}

AssetPath::AssetPath(const AssetPath &cpy) : AssetPath(cpy.toString()) {
}

std::string AssetPath::prefix() const {
    auto str = std::string();
    for (auto &p : path) {
        str += p + ASSET_PATH_SEPARATOR;
    }
    return str;
}
}  // namespace ICE