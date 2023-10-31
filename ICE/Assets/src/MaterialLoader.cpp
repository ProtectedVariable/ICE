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
        //TODO
        mtl->setSources(files);
        return mtl;
    }
}
