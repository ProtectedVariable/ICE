//
// Created by Thomas Ibanez on 16.11.20.
//

#include "RenderComponent.h"

namespace ICE {
    RenderComponent::RenderComponent(Mesh* mesh, Material* material) : mesh(mesh), material(material) {}

    Mesh* RenderComponent::getMesh() const {
        return mesh;
    }

    Material* RenderComponent::getMaterial() const {
        return material;
    }
}