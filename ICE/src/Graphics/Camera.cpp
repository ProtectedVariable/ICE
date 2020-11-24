//
// Created by Thomas Ibanez on 21.11.20.
//

#include "Camera.h"
#include <Util/ICEMath.h>
#include <iostream>

namespace ICE {

    Camera::Camera(const CameraParameters &parameters) : parameters(parameters), projection(Eigen::Matrix4f()) {
        if(parameters.type == Orthographic) {
            projection.setZero();
            projection(0, 0) = 2.0f/(parameters.intrinsic[ICE_CAMERA_RIGHT] - parameters.intrinsic[ICE_CAMERA_LEFT]);
            projection(1, 1) = 2.0f/(parameters.intrinsic[ICE_CAMERA_TOP] - parameters.intrinsic[ICE_CAMERA_BOTTOM]);
            projection(2, 2) = -2.0f/(parameters.intrinsic[ICE_CAMERA_FAR] - parameters.intrinsic[ICE_CAMERA_NEAR]);
            projection(0, 3) = -(parameters.intrinsic[ICE_CAMERA_RIGHT] + parameters.intrinsic[ICE_CAMERA_LEFT])/(parameters.intrinsic[ICE_CAMERA_RIGHT] - parameters.intrinsic[ICE_CAMERA_LEFT]);
            projection(1, 3) = -(parameters.intrinsic[ICE_CAMERA_TOP] + parameters.intrinsic[ICE_CAMERA_BOTTOM])/(parameters.intrinsic[ICE_CAMERA_TOP] - parameters.intrinsic[ICE_CAMERA_BOTTOM]);
            projection(2, 3) = -(parameters.intrinsic[ICE_CAMERA_FAR] + parameters.intrinsic[ICE_CAMERA_NEAR])/(parameters.intrinsic[ICE_CAMERA_FAR] - parameters.intrinsic[ICE_CAMERA_NEAR]);
            projection(3, 3) = 1;
        } else if(parameters.type == Perspective) {
            projection.setZero();
            float fov = DEG_TO_RAD(parameters.intrinsic[ICE_CAMERA_FOV]);
            float ys = 1.0f / (tanf(fov / 2.0f));
            float xs = ys / parameters.intrinsic[ICE_CAMERA_AR];
            projection(0, 0) = xs;
            projection(1, 1) = ys;
            projection(2, 2) = -(parameters.intrinsic[ICE_CAMERA_FAR] + parameters.intrinsic[ICE_CAMERA_NEAR]) / (parameters.intrinsic[ICE_CAMERA_FAR] - parameters.intrinsic[ICE_CAMERA_NEAR]);
            projection(3, 2) = -1;
            projection(2, 3) = -(2*parameters.intrinsic[ICE_CAMERA_FAR]*parameters.intrinsic[ICE_CAMERA_NEAR]) / (parameters.intrinsic[ICE_CAMERA_FAR] - parameters.intrinsic[ICE_CAMERA_NEAR]);
        }
    }

    const Eigen::Matrix4f &Camera::getProjection() const {
        return projection;
    }

    Eigen::Matrix4f Camera::lookThrough(Eigen::Vector3f position, Eigen::Vector3f rotation) {
        auto viewMatrix = translationMatrix(-position);
        viewMatrix = rotationMatrix(-rotation) * viewMatrix;
        return viewMatrix;
    }
}