//
// Created by Thomas Ibanez on 01.08.21.
//

#include <Material.h>
#include "MaterialLoader.h"
#include <json/json.h>
#include <fstream>
#include <JsonParser.h>

using json = nlohmann::json;

namespace ICE {
std::shared_ptr<Material> MaterialLoader::load(const std::vector<std::filesystem::path> &files) {
        json j;
        std::ifstream infile = std::ifstream(files[0]);
        infile >> j;
        infile.close();

        auto mtl = std::make_shared<Material>();
        mtl->setShader(j["shader_id"]);

        for (const auto& data : j["uniforms"]) {
            if (data["type"] == "int") {
                mtl->setUniform<int>(data["name"], data["value"]);
            } else if (data["type"] == "float") {
                mtl->setUniform<float>(data["name"], data["value"]);
            }
        }

        mtl->setSources(files);
        return mtl;
    }
}
