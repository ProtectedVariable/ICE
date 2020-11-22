//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Material.h"

ICE::Material::Material(Shader *shader) : shader(shader) {}

ICE::Shader *ICE::Material::getShader() const {
    return shader;
}
