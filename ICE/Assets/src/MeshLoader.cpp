//
// Created by Thomas Ibanez on 31.07.21.
//

#include "MeshLoader.h"
#include <Util/OBJLoader.h>

namespace ICE {
    Resource* MeshLoader::load(const std::vector<std::string> &file) {
        return new Resource(OBJLoader::loadFromOBJ(file[0]), file);
    }
}
