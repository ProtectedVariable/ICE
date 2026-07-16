//
// Created by Thomas Ibanez on 03.08.21.
//

#include "AssetPath.h"

#include <ICEException.h>
#include <Material.h>
#include <Model.h>
#include <Shader.h>
#include <Texture.h>

namespace ICE {

std::unordered_map<std::type_index, std::string> AssetPath::typenames = {{typeid(Texture2D), "Textures"},
                                                                         {typeid(TextureCube), "CubeMaps"},
                                                                         {typeid(Mesh), "Meshes"},
                                                                         {typeid(Model), "Models"},
                                                                         {typeid(Material), "Materials"},
                                                                         {typeid(Shader), "Shaders"}};

std::unordered_map<std::string, std::type_index> AssetPath::prefixes = {{"Textures", typeid(Texture2D)},
                                                                        {"CubeMaps", typeid(TextureCube)},
                                                                        {"Meshes", typeid(Mesh)},
                                                                        {"Models", typeid(Model)},
                                                                        {"Materials", typeid(Material)},
                                                                        {"Shaders", typeid(Shader)}};

void AssetPath::registerType(std::type_index type, const std::string &prefix) {
    if (auto type_it = typenames.find(type); type_it != typenames.end()) {
        // Already registered: identical prefix is a harmless no-op; a different prefix is a conflict.
        if (type_it->second != prefix) {
            throw ICEException("Asset type already registered with a different prefix");
        }
        return;
    }
    if (auto prefix_it = prefixes.find(prefix); prefix_it != prefixes.end() && prefix_it->second != type) {
        throw ICEException("Asset path prefix '" + prefix + "' is already registered for a different type");
    }
    typenames.emplace(type, prefix);
    prefixes.emplace(prefix, type);
}

std::optional<std::type_index> AssetPath::typeForPrefix(const std::string &prefix) {
    if (auto it = prefixes.find(prefix); it != prefixes.end()) {
        return it->second;
    }
    return std::nullopt;
}

AssetPath::AssetPath(std::string path) {
    size_t last = 0;
    for (size_t i = 0; i < path.length(); i++) {
        if (path[i] == ASSET_PATH_SEPARATOR) {
            this->path.push_back(path.substr(last, i - last));
            last = i + 1;
        }
    }
    name = path.substr(last, path.length() - last);
    m_string = prefix() + name;  // canonical form, computed once
}

const std::string& AssetPath::toString() const {
    return m_string;
}

std::vector<std::string> AssetPath::getPath() const {
    return path;
}

std::string AssetPath::getName() const {
    return name;
}

void AssetPath::setName(const std::string &name) {
    AssetPath::name = name;
    m_string = prefix() + name;  // keep the canonical string in sync
}

std::string AssetPath::prefix() const {
    auto str = std::string();
    for (auto &p : path) {
        str += p + ASSET_PATH_SEPARATOR;
    }
    return str;
}
}  // namespace ICE