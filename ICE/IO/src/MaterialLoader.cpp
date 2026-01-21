//
// Created by Thomas Ibanez on 01.08.21.
//

#include "MaterialLoader.h"

#include <JsonParser.h>
#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::json;

namespace ICE {
std::shared_ptr<Material> MaterialLoader::load(const std::vector<std::filesystem::path> &files) {
    json j;
    std::ifstream infile = std::ifstream(files[0]);
    infile >> j;
    infile.close();

    bool transparent = false;
    if (j.contains("transparent")) {
        transparent = j["transparent"];
    }

    auto mtl = std::make_shared<Material>(transparent);
    mtl->setShader(j["shader_id"]);

    for (const auto &data : j["uniforms"]) {
        if (data["type"] == "int") {
            mtl->setUniform<int>(data["name"], data["value"]);
        } else if (data["type"] == "float") {
            mtl->setUniform<float>(data["name"], data["value"]);
        } else if (data["type"] == "assetUID") {
            mtl->setUniform<AssetUID>(data["name"], data["value"]);
        } else if (data["type"] == "vec3") {
            mtl->setUniform<Eigen::Vector3f>(data["name"], Eigen::Vector3f(data["value"][0], data["value"][1], data["value"][2]));
        } else if (data["type"] == "vec4") {
            mtl->setUniform<Eigen::Vector4f>(data["name"], Eigen::Vector4f(data["value"][0], data["value"][1], data["value"][2], data["value"][3]));
        } else if (data["type"] == "mat4") {
            std::vector<float> vec = data["value"];
            Eigen::Matrix4f mat;
            for (int i = 0; i < 16; i++) {
                mat(i % 4, i / 4) = vec[i];
            }
            mtl->setUniform<Eigen::Matrix4f>(data["name"], mat);
        }
    }

    mtl->setSources(files);
    return mtl;
}
}  // namespace ICE
