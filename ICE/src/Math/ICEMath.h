//
// Created by Thomas Ibanez on 23.11.20.
//

#ifndef ICE_ICEMATH_H
#define ICE_ICEMATH_H
#define _USE_MATH_DEFINES
#include <math.h>
#include <Eigen/Dense>

namespace ICE {
    #define DEG_TO_RAD(x) ((x) * M_PI / 180.0)

    Eigen::Matrix4f rotationMatrix(Eigen::Vector3f angles);
    Eigen::Matrix4f translationMatrix(Eigen::Vector3f translation);
    Eigen::Matrix4f scaleMatrix(Eigen::Vector3f scale);
}

#endif //ICE_ICEMATH_H
