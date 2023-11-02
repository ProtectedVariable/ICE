//
// Created by Thomas Ibanez on 11.12.20.
//

#ifndef ICE_ENGINECONFIG_H
#define ICE_ENGINECONFIG_H

#include "Project.h"
#include <vector>
#define ICE_CONFIG_FILE ".ice_config"

namespace ICE {
    class EngineConfig {
    public:
        EngineConfig();

        static EngineConfig LoadFromFile();
        std::vector<Project>* getLocalProjects();
        Project* getProjectAt(int id);

        void save();

    private:
        std::vector<Project> localProjects;
    };
}

#endif //ICE_ENGINECONFIG_H
