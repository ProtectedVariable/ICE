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
    }
}

void Material::removeUniform(const std::string& name) {
    if (m_uniforms.contains(name)) {
        m_uniforms.erase(name);
    }
}

std::unordered_map<std::string, UniformValue> Material::getAllUniforms() const {
    return m_uniforms;
}

std::string Material::getTypeName() const {
    return "Material";
}

AssetType Material::getType() const {
    return AssetType::EMaterial;
}
}  // namespace ICE