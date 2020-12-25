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

        TransformComponent(const Eigen::Vector3f &position, const Eigen::Vector3f &rotation,
                           const Eigen::Vector3f &scale);

        Eigen::Vector3f* getPosition() { return &position; };

        Eigen::Vector3f* getRotation() { return &rotation; };

        Eigen::Vector3f* getScale() { return &scale; };

        Eigen::Matrix4f getTransformation() const;
    private:
        Eigen::Vector3f position, rotation, scale;
    };
}


#endif //ICE_TRANSFORMCOMPONENT_H
