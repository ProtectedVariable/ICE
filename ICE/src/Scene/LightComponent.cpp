//
// Created by Thomas Ibanez on 22.11.20.
//

#include "LightComponent.h"

namespace ICE {
    LightComponent::LightComponent(ICE::LightType type, Eigen::Vector3f color) : type(type), color(color) {}

    LightType LightComponent::getType() const {
        return type;
    }

    void LightComponent::setType(LightType type) {
        LightComponent::type = type;
    }

    Eigen::Vector3f &LightComponent::getColor() {
        return color;
    }

    void LightComponent::setColor(const Eigen::Vector3f &color) {
        LightComponent::color = color;
    }
}
