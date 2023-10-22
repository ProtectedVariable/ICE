//
// Created by Thomas Ibanez on 31.07.21.
//

#include "MeshLoader.h"

#include <OBJLoader.h>

namespace ICE {
std::shared_ptr<Mesh> MeshLoader::load(const std::vector<std::filesystem::path> &file) {
    auto mesh = std::shared_ptr<Mesh>(OBJLoader::loadFromOBJ(file[0].string()));
    mesh->setSources(file);
    return mesh;
}
}  // namespace ICE
