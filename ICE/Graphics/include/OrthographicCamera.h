#pragma once

#include <Eigen/Dense>

#include "Camera.h"

namespace ICE {

class OrthographicCamera : public Camera {
   public:
    OrthographicCamera(double left, double right, double top, double bottom, double near_, double far_);

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
    double m_left;
    double m_right;
    double m_top;
    double m_bottom;
    double m_near;
    double m_far;
    Eigen::Matrix4f projection;
    Eigen::Vector3f position;
    Eigen::Vector3f rotation;
};
}  // namespace ICE
