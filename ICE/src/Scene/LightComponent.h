//
// Created by Thomas Ibanez on 22.11.20.
//

#ifndef ICE_LIGHTCOMPONENT_H
#define ICE_LIGHTCOMPONENT_H

#include <Eigen/Dense>
#include "Component.h"

namespace ICE {
    enum LightType {
        PointLight,
        DirectionalLight,
        SpotLight
    };

    struct LightComponent : public Component {
        LightType type;
        Eigen::Vector3f color;
    };
}


#endif //ICE_LIGHTCOMPONENT_H
