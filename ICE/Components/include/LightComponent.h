//
// Created by Thomas Ibanez on 22.11.20.
//

#ifndef ICE_LIGHTCOMPONENT_H
#define ICE_LIGHTCOMPONENT_H

#include <Eigen/Dense>

#include "Component.h"

namespace ICE {
enum LightType { PointLight, DirectionalLight, SpotLight };

struct LightComponent : public Component {
    LightComponent(LightType t, const Eigen::Vector3f &col) : type(t), color(col) {}
    LightType type;
    Eigen::Vector3f color;
};
}  // namespace ICE

#endif  //ICE_LIGHTCOMPONENT_H
