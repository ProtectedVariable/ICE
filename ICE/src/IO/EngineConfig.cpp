//
// Created by Thomas Ibanez on 11.12.20.
//

#include "EngineConfig.h"
#include <string>
#include <iostream>
#include <fstream>
#include <Util/Logger.h>

namespace ICE {

    EngineConfig EngineConfig::LoadFromFile() {
        EngineConfigState state;
        auto config = EngineConfig();
        std::string line;
        std::ifstream configFile;
        configFile.open(ICE_CONFIG_FILE);

        if(!configFile.is_open()) {
            Logger::Log(Logger::FATAL, "IO", "Couldn't open config file");
            exit(EXIT_FAILURE);
        }
        while(getline(configFile, line)) {
            if(line == "LOCAL_PROJECTS:") {
                state = PROJECT_LIST;
                continue;
            }

            if(state == PROJECT_LIST) {
                config.localProjects.push_back(Project(line.substr(0, line.find_last_of("/")), line.substr(line.find_last_of("/")+1, line.size())));
            }
        }
        configFile.close();
        return config;
    }

    std::vector<Project>* EngineConfig::getLocalProjects() {
        return &localProjects;
    }

    EngineConfig::EngineConfig() : localProjects(std::vector<Project>()) {}

    void EngineConfig::save() {
        std::ofstream configFile;
        configFile.open(ICE_CONFIG_FILE);
        if(!configFile.is_open()) {
            Logger::Log(Logger::FATAL, "IO", "Couldn't open config file");
            exit(EXIT_FAILURE);
        }
        configFile << "LOCAL_PROJECTS:\n";
        for(auto p : localProjects) {
            configFile << p.getBaseDirectory() << "/" << p.getName() << "\n";
        }
        configFile.close();
    }
}