//
// Created by Thomas Ibanez on 23.11.20.
//

#ifndef ICE_ICEMATH_H
#define ICE_ICEMATH_H
#define _USE_MATH_DEFINES
#include <math.h>

#include <Eigen/Dense>
#include <array>

#define DEG_TO_RAD(x) ((x) *M_PI / 180.0)

#define ICE_CUBEMAP_PX (0)
#define ICE_CUBEMAP_NX (1)
#define ICE_CUBEMAP_PY (2)
#define ICE_CUBEMAP_NY (3)
#define ICE_CUBEMAP_PZ (4)
#define ICE_CUBEMAP_NZ (5)

namespace ICE {

Eigen::Matrix4f rotationMatrix(Eigen::Vector3f angles, bool yaw_first=true);
Eigen::Matrix4f translationMatrix(Eigen::Vector3f translation);
Eigen::Matrix4f scaleMatrix(Eigen::Vector3f scale);
Eigen::Matrix4f transformationMatrix(const Eigen::Vector3f &translation, const Eigen::Vector3f &angles, const Eigen::Vector3f &scale);

Eigen::Vector3f orientation(int face, float x, float y);
int clamp(int x, int a, int b);
std::array<uint8_t *, 6> equirectangularToCubemap(uint8_t *inputPixels, int width, int height, float rotation = 180);
}  // namespace ICE

#endif  //ICE_ICEMATH_H
