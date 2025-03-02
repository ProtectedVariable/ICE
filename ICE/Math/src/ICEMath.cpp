//
// Created by Thomas Ibanez on 25.11.20.
//
#include "ICEMath.h"

namespace ICE {
Eigen::Matrix4f rotationMatrix(Eigen::Vector3f angles, bool yaw_first) {
    auto mx = Eigen::Matrix4f();
    mx.setIdentity();
    float rx = DEG_TO_RAD(angles.x());
    mx(1, 1) = cosf(rx);
    mx(1, 2) = -sinf(rx);
    mx(2, 1) = sinf(rx);
    mx(2, 2) = cosf(rx);

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

    if (yaw_first) {
        return my * mx * mz;
    } else {
        return mx * my * mz;
    }
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

Eigen::Matrix4f transformationMatrix(const Eigen::Vector3f &translation, const Eigen::Vector3f &angles, const Eigen::Vector3f &scale) {
    return translationMatrix(translation) * rotationMatrix(angles) * scaleMatrix(scale);
}

Eigen::Vector3f orientation(int face, float x, float y) {
    Eigen::Vector3f out;
    if (face == ICE_CUBEMAP_PZ) {
        out.x() = -1;
        out.y() = -x;
        out.z() = -y;
    } else if (face == ICE_CUBEMAP_NZ) {
        out.x() = 1;
        out.y() = x;
        out.z() = -y;
    } else if (face == ICE_CUBEMAP_PX) {
        out.x() = x;
        out.y() = -1;
        out.z() = -y;
    } else if (face == ICE_CUBEMAP_NX) {
        out.x() = -x;
        out.y() = 1;
        out.z() = -y;
    } else if (face == ICE_CUBEMAP_NY) {
        out.x() = -y;
        out.y() = -x;
        out.z() = 1;
    } else if (face == ICE_CUBEMAP_PY) {
        out.x() = y;
        out.y() = -x;
        out.z() = -1;
    }
    return out;
}

int clamp(int x, int a, int b) {
    if (x < a)
        return a;
    if (x > b)
        return b;
    return x;
}

std::array<uint8_t *, 6> equirectangularToCubemap(uint8_t *inputPixels, int width, int height, float rotation) {
    auto faceWidth = width / 4;
    auto faceHeight = faceWidth;
    std::array<uint8_t *, 6> outputPixels;

    for (int i = 0; i < 6; i++) {
        int bsize = 3 * faceWidth * faceHeight;
        outputPixels[i] = new uint8_t[bsize];
        for (int x = 0; x < faceWidth; x++) {
            for (int y = 0; y < faceHeight; y++) {
                int to = 3 * ((faceHeight - 1 - y) * faceWidth + x);
                Eigen::Vector3f cube = orientation(i, (2 * (x + 0.5) / faceWidth - 1), (2 * (y + 0.5) / faceHeight - 1));

                auto r = cube.norm();
                auto lon = fmod(atan2(cube.y(), cube.x()) + rotation, 2 * M_PI);
                auto lat = acos(cube.z() / r);

                int fx = width * lon / M_PI / 2 - 0.5;
                int fy = height * lat / M_PI - 0.5;
                fx = clamp(fx, 0, width - 1);
                fy = clamp(fy, 0, height - 1);
                for (int chan = 0; chan < 3; chan++) {
                    outputPixels[i][to + chan] = inputPixels[3 * (fy * width + fx) + chan];
                }
            }
        }
    }
    return outputPixels;
}

std::array<Eigen::Vector4f, 6> extractFrustumPlanes(const Eigen::Matrix4f &PV) {
    std::array<Eigen::Vector4f, 6> planes;
    // Left
    planes[0] = {PV(3, 0) + PV(0, 0), PV(3, 1) + PV(0, 1), PV(3, 2) + PV(0, 2), PV(3, 3) + PV(0, 3)};
    // Right
    planes[1] = {PV(3, 0) - PV(0, 0), PV(3, 1) - PV(0, 1), PV(3, 2) - PV(0, 2), PV(3, 3) - PV(0, 3)};
    // Bottom
    planes[2] = {PV(3, 0) + PV(1, 0), PV(3, 1) + PV(1, 1), PV(3, 2) + PV(1, 2), PV(3, 3) + PV(1, 3)};
    // Top
    planes[3] = {PV(3, 0) - PV(1, 0), PV(3, 1) - PV(1, 1), PV(3, 2) - PV(1, 2), PV(3, 3) - PV(1, 3)};
    // Near
    planes[4] = {PV(3, 0) + PV(2, 0), PV(3, 1) + PV(2, 1), PV(3, 2) + PV(2, 2), PV(3, 3) + PV(2, 3)};
    // Far
    planes[5] = {PV(3, 0) - PV(2, 0), PV(3, 1) - PV(2, 1), PV(3, 2) - PV(2, 2), PV(3, 3) - PV(2, 3)};
    // Normalize planes
    for (int i = 0; i < 6; i++) {
        float length = sqrt(planes[i](0) * planes[i](0) + planes[i](1) * planes[i](1) + planes[i](2) * planes[i](2));
        planes[i](0) /= length;
        planes[i](1) /= length;
        planes[i](2) /= length;
        planes[i](3) /= length;
    }
    return planes;
}

bool isAABBInFrustum(const std::array<Eigen::Vector4f, 6> &frustum, const AABB &aabb) {
    for (int i = 0; i < 6; i++) {
        Eigen::Vector3f positive(frustum[i](0) >= 0 ? aabb.getMax().x() : aabb.getMin().x(),
                                 frustum[i](1) >= 0 ? aabb.getMax().y() : aabb.getMin().y(),
                                 frustum[i](2) >= 0 ? aabb.getMax().z() : aabb.getMin().z());

        if (frustum[i](0) * positive.x() + frustum[i](1) * positive.y() + frustum[i](2) * positive.z() + frustum[i](3) < 0) {
            return false;
        }
    }
    return true;
}
}  // namespace ICE
