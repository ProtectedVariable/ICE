//
// Created by Thomas Ibanez on 09.12.20.
//

#include "Project.h"

namespace ICE {
    Project::Project(const std::string &baseDirectory, const std::string &name) : baseDirectory(baseDirectory),
                                                                                  name(name) {}

    bool Project::CreateDirectories() {
        return false;
    }
}