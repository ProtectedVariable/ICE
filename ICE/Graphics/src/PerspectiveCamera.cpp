//
// Created by Thomas Ibanez on 21.11.20.
//

#include "PerspectiveCamera.h"

#include <ICEMath.h>

#include <iostream>

namespace ICE {

PerspectiveCamera::PerspectiveCamera(double fov_degrees, double aspect_ratio, double near, double far)
    : m_fov(fov_degrees),
      m_aspect_ratio(aspect_ratio),
      m_near(near),
      m_far(far) {
    this->position = Eigen::Vector3f();
    this->rotation = Eigen::Vector3f();
    this->position.setZero();
    this->rotation.setZero();

    //Setup projection matrix
    projection.setZero();
    float fov = DEG_TO_RAD(fov_degrees);
    float ys = 1.0f / (tanf(fov / 2.0f));
    float xs = ys / aspect_ratio;
    projection(0, 0) = xs;
    projection(1, 1) = ys;
    projection(2, 2) = -(far + near) / (far - near);
    projection(3, 2) = -1;
    projection(2, 3) = -(2 * far * near) / (far - near);
}

Eigen::Matrix4f PerspectiveCamera::lookThrough() {
    auto viewMatrix = translationMatrix(-position);
    viewMatrix = rotationMatrix(-rotation) * viewMatrix;
    return viewMatrix;
}

void PerspectiveCamera::forward(float delta) {
    position.x() -= delta * sinf(DEG_TO_RAD(rotation.y()));
    position.z() -= delta * cosf(DEG_TO_RAD(rotation.y()));
}

void PerspectiveCamera::backward(float delta) {
    forward(-delta);
}

void PerspectiveCamera::left(float delta) {
    position.x() += delta * sinf(DEG_TO_RAD(rotation.y() - 90));
    position.z() += delta * cosf(DEG_TO_RAD(rotation.y() - 90));
}

void PerspectiveCamera::right(float delta) {
    left(-delta);
}

void PerspectiveCamera::up(float delta) {
    position.y() += delta;
}

void PerspectiveCamera::down(float delta) {
    up(-delta);
}

void PerspectiveCamera::pitch(float delta) {
    rotation.x() += delta;
}

void PerspectiveCamera::yaw(float delta) {
    rotation.y() += delta;
}

void PerspectiveCamera::roll(float delta) {
    rotation.z() += delta;
}

void PerspectiveCamera::resize(float width, float height) {
    float fov = DEG_TO_RAD(m_fov);
    float ys = 1.0f / (tanf(fov / 2.0f));
    float xs = ys / (width / height);
    projection(0, 0) = xs;
}

Eigen::Vector3f PerspectiveCamera::getPosition() const {
    return position;
}

Eigen::Vector3f PerspectiveCamera::getRotation() const {
    return rotation;
}

Eigen::Matrix4f PerspectiveCamera::getProjection() const {
    return projection;
}

}  // namespace ICE