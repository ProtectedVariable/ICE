//
// Created by Thomas Ibanez on 16.11.20.
//

#include "TransformComponent.h"
#include "Component.h"

namespace ICE {

    TransformComponent::TransformComponent():
            position(Eigen::Vector3d(0, 0, 0)),
            rotation(Eigen::Vector3d(0, 0, 0)),
            scale(Eigen::Vector3d(1,1,1)) {}
}