//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_TRANSFORMCOMPONENT_H
#define ICE_TRANSFORMCOMPONENT_H

#include <Eigen/Dense>
#include "Component.h"

namespace ICE {
    struct TransformComponent : public Component {
        Eigen::Vector3f position, rotation, scale;
    };
}


#endif //ICE_TRANSFORMCOMPONENT_H
