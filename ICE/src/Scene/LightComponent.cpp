//
// Created by Thomas Ibanez on 22.11.20.
//

#include "LightComponent.h"

namespace ICE {
    LightComponent::LightComponent(ICE::LightType type) : type(type) {}

    LightType LightComponent::getType() const {
        return type;
    }

    void LightComponent::setType(LightType type) {
        LightComponent::type = type;
    }
}
