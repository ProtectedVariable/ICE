//
// Created by Thomas Ibanez on 16.11.20.
//

#include "RenderComponent.h"

namespace ICE {
    RenderComponent::RenderComponent(const Mesh* mesh, const Material* material) : mesh(mesh), material(material) {}

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
}