//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_TRANSFORMCOMPONENT_H
#define ICE_TRANSFORMCOMPONENT_H

#include <Eigen/Dense>
#include "Component.h"

namespace ICE {
    class TransformComponent : public Component {
    public:
        TransformComponent();

        const Eigen::Vector3f* getPosition() const { return &position; };

        const Eigen::Vector3f* getRotation() const { return &rotation; };

        const Eigen::Vector3f* getScale() const { return &scale; };

    private:
        Eigen::Vector3f position, rotation, scale;
    };
}


#endif //ICE_TRANSFORMCOMPONENT_H
