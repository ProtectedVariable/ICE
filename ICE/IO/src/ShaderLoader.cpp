//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ShaderLoader.h"

#include <ICEException.h>
#include <Shader.h>

#include <fstream>
#include <sstream>

namespace ICE {
std::shared_ptr<Shader> ShaderLoader::load(const std::vector<std::filesystem::path> &files) {
    if (files.size() < 2) {
        throw ICEException("Shaders must have at least 2 files");
    }

    std::shared_ptr<Shader> shader;
    if (files.size() == 2) {
        shader = graphics_factory->createShader(readAndResolveIncludes(files[0]), readAndResolveIncludes(files[1]));
    } else if (files.size() == 3) {
        shader = graphics_factory->createShader(readAndResolveIncludes(files[0]), readAndResolveIncludes(files[1]), readAndResolveIncludes(files[2]));
    } else {
        throw ICEException("Too many files for shader");
    }

    shader->setSources(files);
    return shader;
}

std::string ShaderLoader::readAndResolveIncludes(const std::filesystem::path &file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + file_path.string());
    }

    std::stringstream result;
    std::string line;

    while (std::getline(file, line)) {
        // Trim whitespace
        auto trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

        if (trimmed.starts_with("#include")) {
            size_t startQuote = trimmed.find('"');
            size_t endQuote = trimmed.find('"', startQuote + 1);
            if (startQuote != std::string::npos && endQuote != std::string::npos) {
                std::filesystem::path includePath = file_path.parent_path() / trimmed.substr(startQuote + 1, endQuote - startQuote - 1);
                result << readAndResolveIncludes(includePath) << "\n";
                continue;
            }
        }
        result << line << "\n";
    }

    return result.str();
}

}  // namespace ICE