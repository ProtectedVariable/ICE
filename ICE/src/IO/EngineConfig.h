//
// Created by Thomas Ibanez on 11.12.20.
//

#ifndef ICE_ENGINECONFIG_H
#define ICE_ENGINECONFIG_H

#include "Project.h"
#include <vector>
#define ICE_CONFIG_FILE ".ice_config"

namespace ICE {
    enum EngineConfigState {
        PROJECT_LIST, END
    };

    class EngineConfig {
    public:
        static EngineConfig LoadFromFile(Camera* camera);
        std::vector<Project>* getLocalProjects();

        EngineConfig();
        void save();

    private:
        std::vector<Project> localProjects;
    };
}

#endif //ICE_ENGINECONFIG_H
