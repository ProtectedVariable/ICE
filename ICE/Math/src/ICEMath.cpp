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

void decomposeMatrix(const Eigen::Matrix4f &M, Eigen::Vector3f &position, Eigen::Vector3f &rotation_eulers, Eigen::Vector3f &scale) {
    // --- 1. Extract Translation (Position) ---
    position = M.block<3, 1>(0, 3);

    // --- 2. Extract Scale and Rotation ---

    // The 3x3 rotational/scaling block (top-left)
    Eigen::Matrix3f R_S = M.block<3, 3>(0, 0);

    // --- Extract Scale (S) ---
    // Scale factors are the magnitudes (norms) of the basis vectors (columns)
    scale.x() = R_S.col(0).norm();
    scale.y() = R_S.col(1).norm();
    scale.z() = R_S.col(2).norm();

    // Check for negative scale (reflection)
    if (R_S.determinant() < 0.0f) {
        scale.x() *= -1.0f;  // Negate one component to account for reflection
    }

    // --- Extract Rotation (R) ---

    // Create the pure rotation matrix R by dividing the R_S block by the extracted scale.
    Eigen::Matrix3f R = R_S;

    // Divide each column by its corresponding scale factor to normalize it to a unit vector.
    if (scale.x() != 0.0f)
        R.col(0) /= scale.x();
    if (scale.y() != 0.0f)
        R.col(1) /= scale.y();
    if (scale.z() != 0.0f)
        R.col(2) /= scale.z();

    // Convert the resulting 3x3 rotation matrix (R) to Z-Y-X Euler angles.
    // Eigen's toEulerAngles(Z, Y, X) returns (alpha, beta, gamma) where:
    // alpha = Rotation around Z (Yaw)
    // beta  = Rotation around Y (Pitch)
    // gamma = Rotation around X (Roll)
    rotation_eulers = R.eulerAngles(2, 1, 0);  // 2=Z, 1=Y, 0=X

    // Note: The returned angles are in radians.

    // The Euler angles returned are in the following order based on the specified Eigen constants:
    // rotation_eulers.x() is the Z-axis rotation (Yaw)
    // rotation_eulers.y() is the Y-axis rotation (Pitch)
    // rotation_eulers.z() is the X-axis rotation (Roll)
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
                // rotation is in degrees; it was being added straight to a radian longitude.
                // Also wrap negative longitudes into [0, 2pi) so they don't clamp to column 0.
                auto lon = fmod(atan2(cube.y(), cube.x()) + DEG_TO_RAD(rotation), 2 * M_PI);
                if (lon < 0) {
                    lon += 2 * M_PI;
                }
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

Frustum extractFrustumPlanes(const Eigen::Matrix4f &PV) {
    Frustum frustum;
    auto extract = [](const Eigen::Vector4f &p) {
        Plane plane;

        Eigen::Vector3f n = p.head<3>();
        float length = n.norm();

        plane.normal = n / length;
        plane.distance = p[3] / length;
        plane.absNormal = plane.normal.cwiseAbs();

        return plane;
    };

    frustum.planes[0] = extract(PV.row(3) + PV.row(0));  // Left
    frustum.planes[1] = extract(PV.row(3) - PV.row(0));  // Right
    frustum.planes[2] = extract(PV.row(3) + PV.row(1));  // Bottom
    frustum.planes[3] = extract(PV.row(3) - PV.row(1));  // Top
    frustum.planes[4] = extract(PV.row(3) + PV.row(2));  // Near
    frustum.planes[5] = extract(PV.row(3) - PV.row(2));  // Far

    return frustum;
}

bool isAABBInFrustum(const Frustum &frustum, const AABB &aabb) {
    return isAABBInFrustum(frustum, aabb.getCenter(), aabb.getExtent());
}

bool isAABBInFrustum(const Frustum &frustum, const Eigen::Vector3f &center, const Eigen::Vector3f &extents) {
    for (int i = 0; i < 6; ++i) {
        const Plane &plane = frustum.planes[i];

        const Eigen::Vector3f &n = plane.absNormal;

        float r = extents.x() * n.x() + extents.y() * n.y() + extents.z() * n.z();

        float s = plane.normal.dot(center) + plane.distance;

        if (s + r < 0.0f)
            return false;
    }

    return true;  // At least partially inside
}
}  // namespace ICE
