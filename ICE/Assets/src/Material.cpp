//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Material.h"

namespace ICE {

Material::Material(bool transparent) : m_transparent(transparent) {
}

AssetUID Material::getShader() const {
    return m_shader;
}

void Material::setShader(AssetUID shader_id) {
    m_shader = shader_id;
}

bool Material::isTransparent() const {
    return m_transparent;
}

void Material::renameUniform(const std::string& previous_name, const std::string& new_name) {
    if (m_uniforms.contains(previous_name)) {
        auto& val = m_uniforms[previous_name];
        m_uniforms.try_emplace(new_name, val);
        m_uniforms.erase(previous_name);
        m_bindings_dirty = true;
    }
}

void Material::removeUniform(const std::string& name) {
    if (m_uniforms.contains(name)) {
        m_uniforms.erase(name);
        m_bindings_dirty = true;
    }
}

const std::unordered_map<std::string, UniformValue>& Material::getAllUniforms() const {
    return m_uniforms;
}

const std::vector<UniformBinding>& Material::getUniformBindings() const {
    if (m_bindings_dirty) {
        m_bindings.clear();
        m_bindings.reserve(m_uniforms.size());
        for (const auto& [name, value] : m_uniforms) {
            UniformKind kind;
            if (std::holds_alternative<AssetUID>(value)) {
                kind = UniformKind::Texture;
            } else if (std::holds_alternative<int>(value)) {
                kind = UniformKind::Int;
            } else if (std::holds_alternative<float>(value)) {
                kind = UniformKind::Float;
            } else if (std::holds_alternative<Eigen::Vector2f>(value)) {
                kind = UniformKind::Vec2;
            } else if (std::holds_alternative<Eigen::Vector3f>(value)) {
                kind = UniformKind::Vec3;
            } else if (std::holds_alternative<Eigen::Vector4f>(value)) {
                kind = UniformKind::Vec4;
            } else {
                kind = UniformKind::Mat4;  // the only remaining alternative
            }
            m_bindings.push_back({name, kind, value});
        }
        m_bindings_dirty = false;
    }
    return m_bindings;
}

std::string Material::getTypeName() const {
    return "Material";
}

AssetType Material::getType() const {
    return AssetType::EMaterial;
}
}  // namespace ICE