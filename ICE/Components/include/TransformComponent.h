//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_TRANSFORMCOMPONENT_H
#define ICE_TRANSFORMCOMPONENT_H

#include <Eigen/Dense>

#include "Component.h"

namespace ICE {
struct TransformComponent : public Component {
    TransformComponent(const Eigen::Vector3f &pos, const Eigen::Vector3f &rot, const Eigen::Vector3f &sca) : position(pos), rotation(rot), scale(sca) {}
    Eigen::Vector3f position, rotation, scale;
};
}  // namespace ICE

#endif  //ICE_TRANSFORMCOMPONENT_H
