//
// Created by Thomas Ibanez on 21.11.20.
//
#pragma once

#include <Eigen/Dense>

#include "Camera.h"

namespace ICE {

class PerspectiveCamera : public Camera {
   public:
    PerspectiveCamera(double fov, double aspect_ratio, double near_, double far_);

    Eigen::Matrix4f lookThrough() override;

    void forward(float delta) override;
    void backward(float delta) override;
    void left(float delta) override;
    void right(float delta) override;
    void up(float delta) override;
    void down(float delta) override;

    void pitch(float delta) override;
    void yaw(float delta) override;
    void roll(float delta) override;

    void resize(float width, float height) override;

    Eigen::Matrix4f getProjection() const override;
    Eigen::Vector3f getPosition() const override;
    void setPosition(const Eigen::Vector3f &) override;
    Eigen::Vector3f getRotation() const override;

   private:
    double m_fov;
    double m_aspect_ratio;
    double m_near;
    double m_far;
    float zoom;
    Eigen::Matrix4f projection;
    Eigen::Vector3f position;
    Eigen::Vector3f rotation;
};
}  // namespace ICE
