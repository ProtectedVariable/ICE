//
// Created by Thomas Ibanez on 09.12.20.
//

#include "Project.h"
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#else
  error "Missing the <filesystem> header."
#endif

namespace ICE {
    Project::Project(const std::string &baseDirectory, const std::string &name) : baseDirectory(baseDirectory),
                                                                                  name(name) {}

    bool Project::CreateDirectories() {
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Meshes");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Materials");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Shaders");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Textures");
        fs::create_directories(baseDirectory + "/" + name + "/Assets/Scripts");
        fs::create_directories(baseDirectory + "/" + name + "/Scenes");
        return true;
    }
}