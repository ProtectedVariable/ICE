//
// Created by Thomas Ibanez on 31.07.21.
//

#include <Shader.h>
#include "ShaderLoader.h"

namespace ICE {
    Resource *ShaderLoader::load(const std::vector<std::string> &files) {
        return new Resource(Shader::Create(files[0], files[1]), files);
    }
}