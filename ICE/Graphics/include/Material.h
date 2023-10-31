//
// Created by Thomas Ibanez on 16.11.20.
//

#pragma once

#include <Eigen/Dense>
#include <variant>

#include "Shader.h"
#include "Texture.h"

namespace ICE {

using UniformValue = std::variant<AssetUID, int, float, Eigen::Vector3f, Eigen::Vector4f, Eigen::Matrix4f>;

class Material : public Asset {
   public:
    Material();

    template<typename T>
    void setUniform(const std::string& name, const T& value) {
        if(!m_uniforms.contains(name)) {
            m_uniforms.try_emplace(name, value);
        } else {
            m_uniforms[name] = value;
        }
    }

    template<typename T>
    T getUniform(const std::string& name) const {
        if (m_uniforms.contains(name)) {
            return std::get<T>(m_uniforms.at(name));
        } else {
            throw std::runtime_error("Uniform not found: " + name);
        }
    }

    std::unordered_map<std::string, UniformValue> getAllUniforms() const;
    AssetUID getShader() const;
    void setShader(AssetUID shader_id);

    //Asset interface
    std::string getTypeName() const override;
    AssetType getType() const override;
    void load() override;
    void unload() override;

   private:
    AssetUID m_shader = NO_ASSET_ID;
    std::unordered_map<std::string, UniformValue> m_uniforms;
};
}  // namespace ICE
