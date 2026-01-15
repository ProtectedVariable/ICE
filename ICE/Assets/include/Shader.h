//
// Created by Thomas Ibanez on 20.11.20.
//

#pragma once

#include <Asset.h>

#include <unordered_map>

namespace ICE {
enum ShaderStage { Vertex, Fragment, Geometry, TessControl, TessEval, Compute };

// Map of shader stages to their source code
using ShaderSource = std::unordered_map<ShaderStage, std::string>;

class Shader : public Asset {
   public:
    Shader() = default;
    Shader(const ShaderSource& sources) : m_sources(sources) {}

    ShaderSource getSources() const { return m_sources; }

    std::string getTypeName() const override;

    AssetType getType() const override;

   private:
    ShaderSource m_sources;
};
}  // namespace ICE