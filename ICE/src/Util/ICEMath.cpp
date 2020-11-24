//
// Created by Thomas Ibanez on 25.11.20.
//
#include "ICEMath.h"

namespace ICE {
    Eigen::Matrix4f rotationMatrix(Eigen::Vector3f angles) {
        auto mx = Eigen::Matrix4f();
        mx.setIdentity();
        mx(1,1) = cosf(angles.x());
        mx(1,2) = -sinf(angles.x());
        mx(2,1) = sinf(angles.x());
        mx(2,2) = cosf(angles.x());

        auto my = Eigen::Matrix4f();
        my.setIdentity();
        my(0, 0) = cosf(angles.y());
        my(0, 2) = sinf(angles.y());
        my(2, 0) = -sinf(angles.y());
        my(2, 2) = cosf(angles.y());

        auto mz = Eigen::Matrix4f();
        mz.setIdentity();
        mz(0, 0) = cosf(angles.z());
        mz(0, 1) = -sinf(angles.z());
        mz(1, 0) = sinf(angles.z());
        mz(1, 1) = cosf(angles.z());

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
}
