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
    Resource *MaterialLoader::load(const std::vector<std::string> &files) {
        json j;
        std::ifstream infile = std::ifstream(files[0]);
        infile >> j;
        infile.close();

        Material* mtl = new Material();
        if(j["type"] == "phong") {
            mtl->setAlbedo(JsonParser::parseVec3(j["albedo"]));
            mtl->setSpecular(JsonParser::parseVec3(j["specular"]));
            mtl->setAmbient(JsonParser::parseVec3(j["ambient"]));
            mtl->setAlpha(j["alpha"]);
            if(j["diffuseMap"] != "null") {
                mtl->setDiffuseMap((j["diffuseMap"]));
            }
            if(j["specularMap"] != "null") {
                mtl->setSpecularMap((j["specularMap"]));
            }
            if(j["ambientMap"] != "null") {
                mtl->setAmbientMap((j["ambientMap"]));
            }
            if(j["normalMap"] != "null") {
                mtl->setNormalMap((j["normalMap"]));
            }
        }
        return new Resource(mtl, files);
    }
}
