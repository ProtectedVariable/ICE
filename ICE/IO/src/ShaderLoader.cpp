//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ShaderLoader.h"

#include <ICEException.h>
#include <Shader.h>

#include "FileUtils.h"

namespace ICE {
std::shared_ptr<Shader> ShaderLoader::load(const std::vector<std::filesystem::path> &files) {
    if (files.size() < 2) {
        throw ICEException("Shaders must have at least 2 files");
    }
    std::shared_ptr<Shader> shader;
    if (files.size() == 2) {
        shader = graphics_factory->createShader(FileUtils::readFile(files[0].string()), FileUtils::readFile(files[1].string()));
    } else if (files.size() == 3) {
        shader = graphics_factory->createShader(FileUtils::readFile(files[0].string()), FileUtils::readFile(files[1].string()),
                                                FileUtils::readFile(files[2].string()));
    } else {
        throw ICEException("Too many files for shader");
    }

    shader->setSources(files);
    return shader;
}
}  // namespace ICE