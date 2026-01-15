#include "ShaderExporter.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <span>
#include <variant>

using json = nlohmann::json;

namespace ICE {

void ShaderExporter::writeToJson(const std::filesystem::path &path, const Shader &shader) {
    std::ofstream outstream;
    outstream.open(path);
    json j = json::array();
    for (const auto& [stage, source] : shader.getSources()) {
        j.push_back({
            {"stage", stageToString(stage)},
            {"file", source}});
    }
    outstream << j.dump(4);
    outstream.close();
}

void ShaderExporter::writeToBin(const std::filesystem::path &path, const Shader &object) {
    throw std::runtime_error("Not implemented");
}
}  // namespace ICE