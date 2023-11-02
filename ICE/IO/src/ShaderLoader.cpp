//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ShaderLoader.h"

#include <Shader.h>

namespace ICE {
std::shared_ptr<Shader> ShaderLoader::load(const std::vector<std::filesystem::path> &files) {
    auto shader = graphics_factory->createShader(files[0].string(), files[1].string());
    shader->setSources(files);
    return shader;
}
}  // namespace ICE