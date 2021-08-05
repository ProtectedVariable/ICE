//
// Created by Thomas Ibanez on 16.11.20.
//

#include "RenderComponent.h"

namespace ICE {
    RenderComponent::RenderComponent(AssetUID mesh, AssetUID material, AssetUID shader) : mesh(mesh), material(material), shader(shader) {}

    AssetUID RenderComponent::getMesh() const {
        return mesh;
    }

    AssetUID RenderComponent::getMaterial() const {
        return material;
    }

    void RenderComponent::setMesh(AssetUID mesh) {
        this->mesh = mesh;
    }

    void RenderComponent::setMaterial(AssetUID material) {
        this->material = material;
    }

    AssetUID RenderComponent::getShader() {
        return shader;
    }

    void RenderComponent::setShader(AssetUID shader) {
        this->shader = shader;
    }
}