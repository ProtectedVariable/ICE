//
// Created by Thomas Ibanez on 21.11.20.
//

#pragma once

#include <Eigen/Dense>

namespace ICE {
enum ProjectionType { Perspective, Orthographic };

class Camera {
   public:
    virtual Eigen::Matrix4f lookThrough() = 0;

    virtual void forward(float delta) = 0;
    virtual void backward(float delta) = 0;
    virtual void left(float delta) = 0;
    virtual void right(float delta) = 0;
    virtual void up(float delta) = 0;
    virtual void down(float delta) = 0;

    virtual void pitch(float delta) = 0;
    virtual void yaw(float delta) = 0;
    virtual void roll(float delta) = 0;

    virtual void resize(float width, float height) = 0;

    virtual Eigen::Matrix4f getProjection() const = 0;
    virtual Eigen::Vector3f getPosition() const = 0;
    virtual void setPosition(const Eigen::Vector3f&) = 0;
    virtual Eigen::Vector3f getRotation() const = 0;

   private:
    float m_zoom;
};
}  // namespace ICE