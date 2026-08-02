//
// Created by Thomas Ibanez on 16.11.20.
//

#pragma once

#include <Eigen/Dense>
#include <string>
#include <variant>
#include <vector>

#include "Shader.h"
#include "Texture.h"

namespace ICE {

using UniformValue = std::variant<AssetUID, int, float, Eigen::Vector2f, Eigen::Vector3f, Eigen::Vector4f, Eigen::Matrix4f>;

// The variant's active alternative, resolved once so the render hot path can switch on it instead
// of walking a std::holds_alternative chain per material bind.
enum class UniformKind { Texture, Int, Float, Vec2, Vec3, Vec4, Mat4 };

struct UniformBinding {
    std::string name;
    UniformKind kind;
    UniformValue value;
};

class Material : public Asset {
   public:
    Material(bool transparent = false);

    template<typename T>
    void setUniform(const std::string& name, const T& value) {
        if (!m_uniforms.contains(name)) {
            m_uniforms.try_emplace(name, value);
        } else {
            m_uniforms[name] = value;
        }
        m_bindings_dirty = true;
    }

    template<typename T>
    T getUniform(const std::string& name) const {
        if (m_uniforms.contains(name)) {
            return std::get<T>(m_uniforms.at(name));
        } else {
            throw std::runtime_error("Uniform not found: " + name);
        }
    }

    void renameUniform(const std::string& previous_name, const std::string& new_name);
    void removeUniform(const std::string& name);

    const std::unordered_map<std::string, UniformValue>& getAllUniforms() const;

    // Pre-classified uniforms for the render hot path (see UniformBinding). Rebuilt lazily the
    // first time it's read after a uniform changes; otherwise returned straight from the cache.
    const std::vector<UniformBinding>& getUniformBindings() const;

    AssetUID getShader() const;
    void setShader(AssetUID shader_id);
    bool isTransparent() const;

    //Asset interface
    std::string getTypeName() const override;
    AssetType getType() const override;

   private:
    AssetUID m_shader = NO_ASSET_ID;
    std::unordered_map<std::string, UniformValue> m_uniforms;
    bool m_transparent;

    // Cache of pre-classified uniforms, rebuilt when m_uniforms changes. Mutable so the read-only
    // accessor can refresh it lazily.
    mutable std::vector<UniformBinding> m_bindings;
    mutable bool m_bindings_dirty = true;
};
}  // namespace ICE
