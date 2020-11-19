//
// Created by Thomas Ibanez on 16.11.20.
//

#include "RenderComponent.h"

namespace ICE {
    RenderComponent::RenderComponent(const Mesh &mesh, const Material &material) : mesh(mesh), material(material) {}
}