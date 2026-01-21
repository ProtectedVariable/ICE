#pragma once

#include <Shader.h>

#include "AssetExporter.h"

namespace ICE {
class ShaderExporter : public AssetExporter<Shader> {
   public:
    void writeToJson(const std::filesystem::path &path, const Shader &object) override;
    void writeToBin(const std::filesystem::path &path, const Shader &object) override;

    constexpr std::string stageToString(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex:
                return "vertex";
            case ShaderStage::Fragment:
                return "fragment";
            case ShaderStage::Geometry:
                return "geometry";
            case ShaderStage::TessControl:
                return "tess_control";
            case ShaderStage::TessEval:
                return "tess_eval";
            case ShaderStage::Compute:
                return "compute";
            default:
                return "unknown";
        }
    }
};
}  // namespace ICE
