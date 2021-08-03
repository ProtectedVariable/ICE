//
// Created by Thomas Ibanez on 03.08.21.
//

#include <Graphics/Texture.h>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>
#include <Graphics/Shader.h>
#include "AssetPath.h"

namespace ICE {

    std::unordered_map<std::type_index, std::string> AssetPath::typenames = {
            {typeid(Texture2D), "Texture"},
            {typeid(TextureCube), "CubeMap"},
            {typeid(Mesh), "Mesh"},
            {typeid(Material), "Material"},
            {typeid(Shader), "Shader"}
    };

    AssetPath::AssetPath(std::string path) {
        size_t last = 0;
        for(size_t i = 0; i < path.length(); i++) {
            if(path[i] == ASSET_PATH_SEPARATOR) {
                this->path.push_back(path.substr(last, i-last));
                last = i + 1;
            }
        }
        name = path.substr(last, path.length()-last);
    }

    std::string AssetPath::toString() const {
        auto str = std::string();
        for(auto &p : path) {
            str += p+ASSET_PATH_SEPARATOR;
        }
        return (str + name);
    }

    const std::vector<std::string> &AssetPath::getPath() const {
        return path;
    }

    const std::string &AssetPath::getName() const {
        return name;
    }
}