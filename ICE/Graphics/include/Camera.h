//
// Created by Thomas Ibanez on 21.11.20.
//

#pragma once

#include <Eigen/Dense>

namespace ICE {
enum ProjectionType { Perspective, Orthographic };

class Camera {
   public:
    virtual ~Camera() = default;
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
    virtual void setRotation(const Eigen::Vector3f &) = 0;

   private:
    float m_zoom;
};

// Fluent, non-owning view over a Camera: lets setup read as a chain --
//   scene.camera().setPosition({0, 5, -5}).pitch(-30);
// Each mutator forwards to the underlying camera and returns *this. Use operator-> / get() to
// reach the rest of the Camera interface (getters, resize, ...). Cheap to copy (a bare pointer);
// owns nothing.
class CameraHandle {
   public:
    explicit CameraHandle(Camera* camera) : m_camera(camera) {}

    CameraHandle& setPosition(const Eigen::Vector3f& p) { m_camera->setPosition(p); return *this; }
    CameraHandle& setRotation(const Eigen::Vector3f& r) { m_camera->setRotation(r); return *this; }
    CameraHandle& pitch(float d) { m_camera->pitch(d); return *this; }
    CameraHandle& yaw(float d) { m_camera->yaw(d); return *this; }
    CameraHandle& roll(float d) { m_camera->roll(d); return *this; }
    CameraHandle& forward(float d) { m_camera->forward(d); return *this; }
    CameraHandle& backward(float d) { m_camera->backward(d); return *this; }
    CameraHandle& left(float d) { m_camera->left(d); return *this; }
    CameraHandle& right(float d) { m_camera->right(d); return *this; }
    CameraHandle& up(float d) { m_camera->up(d); return *this; }
    CameraHandle& down(float d) { m_camera->down(d); return *this; }

    Camera* operator->() const { return m_camera; }
    Camera& operator*() const { return *m_camera; }
    Camera* get() const { return m_camera; }

   private:
    Camera* m_camera;
};
}  // namespace ICE