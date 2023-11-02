//
// Created by Thomas Ibanez on 11.12.20.
//

#include "EngineConfig.h"

#include <Logger.h>
#include <json/json.h>

#include <fstream>
#include <iostream>
#include <string>

namespace ICE {

using json = nlohmann::json;

EngineConfig::EngineConfig() {
}

EngineConfig EngineConfig::LoadFromFile() {
    auto config = EngineConfig();
    std::string line;
    std::ifstream configFile;
    if (std::filesystem::exists(ICE_CONFIG_FILE)) {
        configFile.open(ICE_CONFIG_FILE);

        if (!configFile.is_open()) {
            Logger::Log(Logger::FATAL, "IO", "Couldn't open config file");
            exit(EXIT_FAILURE);
        }

        json j;
        configFile >> j;
        configFile.close();

        for (const auto &project : j["projects"]) {
            std::filesystem::path path = std::string(project["path"]);
            std::string name = project["name"];
            config.localProjects.push_back(Project(path, name));
        }
    }
    return config;
}

std::vector<Project> *EngineConfig::getLocalProjects() {
    return &localProjects;
}

void EngineConfig::save() {
    std::ofstream configFile;
    configFile.open(ICE_CONFIG_FILE);
    if (!configFile.is_open()) {
        Logger::Log(Logger::FATAL, "IO", "Couldn't open config file");
        exit(EXIT_FAILURE);
    }

    json j;
    std::vector<json> entries;
    for (const auto &p : localProjects) {
        json project_json;
        //Exclude last folder as it's the name of the project
        project_json["path"] = p.getBaseDirectory().parent_path().string();
        project_json["name"] = p.getName();
        entries.push_back(project_json);
    }

    j["projects"] = entries;

    configFile << j.dump(4);
    configFile.close();
}

Project *EngineConfig::getProjectAt(int i) {
    return &(localProjects.at(i));
}
}  // namespace ICE