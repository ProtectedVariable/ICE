//
// Created by Thomas Ibanez on 21.11.20.
//

#include "OrthographicCamera.h"

#include <ICEMath.h>

#include <iostream>

namespace ICE {

OrthographicCamera::OrthographicCamera(double left, double right, double top, double bottom, double near_, double far_)
    : m_right(right),
      m_left(left),
      m_top(top),
      m_bottom(bottom),
      m_near(near_),
      m_far(far_) {
    this->position = Eigen::Vector3f();
    this->rotation = Eigen::Vector3f();
    this->position.setZero();
    this->rotation.setZero();

    //Setup projection matrix
    projection.setZero();
    projection(0, 0) = 2.0 / (right - left);
    projection(1, 1) = 2.0 / (top - bottom);
    projection(2, 2) = -2.0 / (far_ - near_);
    projection(0, 3) = -(right + left) / (right - left);
    projection(1, 3) = -(top + bottom) / (top - bottom);
    projection(2, 3) = -(far_ + near_) / (far_ - near_);
    projection(3, 3) = 1.0;
}

Eigen::Matrix4f OrthographicCamera::lookThrough() {
    Eigen::Matrix4f viewMatrix = rotationMatrix(-rotation) * translationMatrix(-position);
    return viewMatrix;
}

void OrthographicCamera::forward(float delta) {
    position.x() -= delta * sinf(DEG_TO_RAD(rotation.y()));
    position.z() -= delta * cosf(DEG_TO_RAD(rotation.y()));
}

void OrthographicCamera::backward(float delta) {
    forward(-delta);
}

void OrthographicCamera::left(float delta) {
    position.x() += delta * sinf(DEG_TO_RAD(rotation.y() - 90));
    position.z() += delta * cosf(DEG_TO_RAD(rotation.y() - 90));
}

void OrthographicCamera::right(float delta) {
    left(-delta);
}

void OrthographicCamera::up(float delta) {
    position.y() += delta;
}

void OrthographicCamera::down(float delta) {
    up(-delta);
}

void OrthographicCamera::pitch(float delta) {
    rotation.x() += delta;
}

void OrthographicCamera::yaw(float delta) {
    rotation.y() += delta;
}

void OrthographicCamera::roll(float delta) {
    rotation.z() += delta;
}

void OrthographicCamera::resize(float width, float height) {
    //TODO
}

Eigen::Vector3f OrthographicCamera::getPosition() const {
    return position;
}

void OrthographicCamera::setPosition(const Eigen::Vector3f &pos) {
    position = pos;
}

Eigen::Vector3f OrthographicCamera::getRotation() const {
    return rotation;
}

void OrthographicCamera::setRotation(const Eigen::Vector3f &rot) {
    rotation = rot;
}

Eigen::Matrix4f OrthographicCamera::getProjection() const {
    return projection;
}

}  // namespace ICE