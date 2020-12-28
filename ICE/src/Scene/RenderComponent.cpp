//
// Created by Thomas Ibanez on 16.11.20.
//

#include "RenderComponent.h"

namespace ICE {
    RenderComponent::RenderComponent(const Mesh* mesh, const Material* material, Shader* shader) : mesh(mesh), material(material), shader(shader) {}

    const Mesh* RenderComponent::getMesh() const {
        return mesh;
    }

    const Material* RenderComponent::getMaterial() const {
        return material;
    }

    void RenderComponent::setMesh(Mesh *mesh) {
        RenderComponent::mesh = mesh;
    }

    void RenderComponent::setMaterial(Material *material) {
        RenderComponent::material = material;
    }

    Shader *RenderComponent::getShader() {
        return shader;
    }

    void RenderComponent::setShader(Shader *shader) {
        RenderComponent::shader = shader;
    }
}