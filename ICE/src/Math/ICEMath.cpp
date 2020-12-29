//
// Created by Thomas Ibanez on 25.11.20.
//
#include "ICEMath.h"

namespace ICE {
    Eigen::Matrix4f rotationMatrix(Eigen::Vector3f angles) {
        auto mx = Eigen::Matrix4f();
        mx.setIdentity();
        float rx = DEG_TO_RAD(angles.x());
        mx(1,1) = cosf(rx);
        mx(1,2) = -sinf(rx);
        mx(2,1) = sinf(rx);
        mx(2,2) = cosf(rx);

        auto my = Eigen::Matrix4f();
        my.setIdentity();
        float ry = DEG_TO_RAD(angles.y());
        my(0, 0) = cosf(ry);
        my(0, 2) = sinf(ry);
        my(2, 0) = -sinf(ry);
        my(2, 2) = cosf(ry);

        auto mz = Eigen::Matrix4f();
        mz.setIdentity();
        float rz = DEG_TO_RAD(angles.z());
        mz(0, 0) = cosf(rz);
        mz(0, 1) = -sinf(rz);
        mz(1, 0) = sinf(rz);
        mz(1, 1) = cosf(rz);

        return mx * my * mz;
    }

    Eigen::Matrix4f translationMatrix(Eigen::Vector3f translation) {
        auto t = Eigen::Matrix4f();
        t.setIdentity();
        t(0, 3) = translation.x();
        t(1, 3) = translation.y();
        t(2, 3) = translation.z();
        return t;
    }

    Eigen::Matrix4f scaleMatrix(Eigen::Vector3f scale) {
        auto t = Eigen::Matrix4f();
        t.setIdentity();
        t(0, 0) = scale.x();
        t(1, 1) = scale.y();
        t(2, 2) = scale.z();
        return t;
    }
}
