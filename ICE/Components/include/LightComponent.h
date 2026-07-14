//
// Created by Thomas Ibanez on 22.11.20.
//

#ifndef ICE_LIGHTCOMPONENT_H
#define ICE_LIGHTCOMPONENT_H

#include <Eigen/Dense>

#include "Component.h"

namespace ICE {
// Short aliases (Point/Directional/Spot) sit alongside the original *Light names; both refer to
// the same values, so existing code keeps compiling.
enum LightType {
    PointLight = 0,
    DirectionalLight = 1,
    SpotLight = 2,
    Point = PointLight,
    Directional = DirectionalLight,
    Spot = SpotLight,
};

struct LightComponent : public Component {
    LightComponent(LightType t, const Eigen::Vector3f &col) : type(t), color(col) {}
    LightType type;
    Eigen::Vector3f color;
    float distance_dropoff = 0;
};
}  // namespace ICE

#endif  //ICE_LIGHTCOMPONENT_H
