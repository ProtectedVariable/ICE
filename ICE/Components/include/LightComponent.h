//
// Created by Thomas Ibanez on 22.11.20.
//

#ifndef ICE_LIGHTCOMPONENT_H
#define ICE_LIGHTCOMPONENT_H

#include <Eigen/Dense>

#include "Component.h"

namespace ICE {
enum LightType { PointLight = 0, DirectionalLight = 1, SpotLight = 2 };

struct LightComponent : public Component {
    LightComponent(LightType t, const Eigen::Vector3f &col) : type(t), color(col) {}
    LightType type;
    Eigen::Vector3f color;
    float distance_dropoff = 0;
};
}  // namespace ICE

#endif  //ICE_LIGHTCOMPONENT_H
