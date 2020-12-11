//
// Created by Thomas Ibanez on 09.12.20.
//

#include "Project.h"
#include <filesystem>

namespace ICE {
    Project::Project(const std::string &baseDirectory, const std::string &name) : baseDirectory(baseDirectory),
                                                                                  name(name) {}

    bool Project::CreateDirectories() {
        std::filesystem::create_directories(baseDirectory + "/" + name + "/Assets/Meshes");
        std::filesystem::create_directories(baseDirectory + "/" + name + "/Assets/Materials");
        std::filesystem::create_directories(baseDirectory + "/" + name + "/Assets/Shaders");
        std::filesystem::create_directories(baseDirectory + "/" + name + "/Assets/Textures");
        std::filesystem::create_directories(baseDirectory + "/" + name + "/Assets/Scripts");
        std::filesystem::create_directories(baseDirectory + "/" + name + "/Scenes");
        return true;
    }
}