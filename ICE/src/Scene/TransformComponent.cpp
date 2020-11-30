//
// Created by Thomas Ibanez on 16.11.20.
//

#include "TransformComponent.h"
#include "Component.h"

#include <Util/ICEMath.h>
namespace ICE {

    TransformComponent::TransformComponent():
            position(Eigen::Vector3f(0, 0, 0)),
            rotation(Eigen::Vector3f(0, 0, 0)),
            scale(Eigen::Vector3f(1,1,1)) {}

    Eigen::Matrix4f TransformComponent::getTransformation() const {
        auto m = translationMatrix(this->position);
        m = m * rotationMatrix(this->rotation);
        m = m * scaleMatrix(this->scale);
        return m;
    }
}