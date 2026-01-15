//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ShaderLoader.h"

#include <ICEException.h>
#include <Shader.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace ICE {
std::shared_ptr<Shader> ShaderLoader::load(const std::vector<std::filesystem::path> &files) {
    if (files.empty()) {
        throw ICEException("No files provided for shader");
    }

    auto json = nlohmann::json::parse(std::ifstream(files[0]));

    ShaderSource shader_sources;
    for (const auto &stage_source : json) {
        auto stage = stageFromString(stage_source["stage"]);
        auto file = stage_source["file"].get<std::string>();
        shader_sources[stage] = readAndResolveIncludes(files[0].parent_path() / file);
    }

    auto shader = std::make_shared<Shader>(shader_sources);

    shader->setSources(files);
    return shader;
}

constexpr ShaderStage ShaderLoader::stageFromString(const std::string &str) {
    if (str == "vertex") {
        return ShaderStage::Vertex;
    } else if (str == "fragment") {
        return ShaderStage::Fragment;
    } else if (str == "geometry") {
        return ShaderStage::Geometry;
    } else if (str == "tess_control") {
        return ShaderStage::TessControl;
    } else if (str == "tess_eval") {
        return ShaderStage::TessEval;
    } else if (str == "compute") {
        return ShaderStage::Compute;
    } else {
        throw ICEException("Unknown shader stage: " + str);
    }
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