//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Material.h"

namespace ICE {

Material::Material() {
}

AssetUID Material::getShader() const {
    return m_shader;
}

void Material::setShader(AssetUID shader_id) {
    m_shader = shader_id;
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
void Material::load() {
}
void Material::unload() {
}
}  // namespace ICE