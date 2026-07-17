#include "SceneCamera.h"

#include <CameraComponent.h>
#include <OrthographicCamera.h>
#include <PerspectiveCamera.h>
#include <Registry.h>
#include <TransformComponent.h>

namespace ICE {

SceneCamera::SceneCamera() {
    rebuildImpl();  // free default
}

void SceneCamera::bindEntity(Registry* registry, Entity entity) {
    m_registry = registry;
    m_entity = entity;
    rebuildImpl();
}

TransformComponent* SceneCamera::transform() const {
    if (!isBound()) {
        return nullptr;
    }
    return m_registry->tryGetComponent<TransformComponent>(m_entity);
}

void SceneCamera::rebuildImpl() {
    // Projection parameters come from the bound CameraComponent; unbound (or an entity without one)
    // uses the historical scene-camera default so existing scenes render identically.
    CameraComponent params;  // defaults == the old default camera
    if (isBound()) {
        if (auto* cc = m_registry->tryGetComponent<CameraComponent>(m_entity)) {
            params = *cc;
        }
    }

    if (params.projection == CameraComponent::Projection::Orthographic) {
        const double half_h = params.ortho_size;
        const double half_w = half_h * m_aspect;
        m_impl = std::make_shared<OrthographicCamera>(-half_w, half_w, half_h, -half_h, params.near_plane, params.far_plane);
    } else {
        m_impl = std::make_shared<PerspectiveCamera>(params.fov, m_aspect, params.near_plane, params.far_plane);
    }
}

Eigen::Matrix4f SceneCamera::lookThrough() {
    auto* tc = transform();
    if (!tc) {
        return m_impl->lookThrough();  // free mode: byte-identical to the legacy scene camera
    }
    // Bound view = the inverse of the entity's world transform (scale stripped so a scaled camera
    // entity can't skew the view). This is the standard convention and it composes correctly
    // through scene-graph parenting, which is what makes a parented camera follow its rig.
    //
    // For the single-axis orientations the engine's cameras use in practice (a pitch, a yaw) this
    // equals the free camera's legacy view exactly; it only diverges for a combined multi-axis
    // orientation, where the legacy formula rotationMatrix(-euler, false) was never a true inverse
    // anyway. Bound cameras are new, so there is no prior view to preserve there.
    Eigen::Matrix4f world = tc->getWorldMatrix();
    Eigen::Vector3f pos = world.block<3, 1>(0, 3);
    Eigen::Matrix3f rot = world.block<3, 3>(0, 0);
    for (int i = 0; i < 3; ++i) {
        const float n = rot.col(i).norm();
        if (n > 0.0f) {
            rot.col(i) /= n;
        }
    }
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view.block<3, 3>(0, 0) = rot.transpose();          // R^-1 for an orthonormal rotation
    view.block<3, 1>(0, 3) = -(rot.transpose() * pos);  // -R^-1 * position
    return view;
}

Eigen::Vector3f SceneCamera::getPosition() const {
    if (auto* tc = transform()) {
        return tc->getWorldMatrix().block<3, 1>(0, 3);  // world-space position (matches lookThrough)
    }
    return m_impl->getPosition();
}

Eigen::Matrix4f SceneCamera::getProjection() const {
    return m_impl->getProjection();
}

Eigen::Vector3f SceneCamera::getRotation() const {
    if (auto* tc = transform()) {
        return tc->getRotationEulerDeg();
    }
    return m_impl->getRotation();
}

void SceneCamera::resize(float width, float height) {
    if (height != 0.0f) {
        m_aspect = width / height;
    }
    m_impl->resize(width, height);
}

template<typename Op>
void SceneCamera::mutate(Op&& op) {
    auto* tc = transform();
    if (!tc) {
        op(*m_impl);  // free mode: operate directly on the camera
        return;
    }
    // Bound: load the entity's local pose, apply the op with the existing camera math, write back.
    // Edits therefore land on the entity's transform and compose through the scene graph. The euler
    // round-trip (getRotationEulerDeg) is authoring-only and off the render path.
    m_impl->setPosition(tc->getPosition());
    m_impl->setRotation(tc->getRotationEulerDeg());
    op(*m_impl);
    tc->setPosition(m_impl->getPosition());
    tc->setRotationEulerDeg(m_impl->getRotation());
}

void SceneCamera::setPosition(const Eigen::Vector3f& p) {
    mutate([&](Camera& c) { c.setPosition(p); });
}
void SceneCamera::setRotation(const Eigen::Vector3f& r) {
    mutate([&](Camera& c) { c.setRotation(r); });
}
void SceneCamera::forward(float d) {
    mutate([&](Camera& c) { c.forward(d); });
}
void SceneCamera::backward(float d) {
    mutate([&](Camera& c) { c.backward(d); });
}
void SceneCamera::left(float d) {
    mutate([&](Camera& c) { c.left(d); });
}
void SceneCamera::right(float d) {
    mutate([&](Camera& c) { c.right(d); });
}
void SceneCamera::up(float d) {
    mutate([&](Camera& c) { c.up(d); });
}
void SceneCamera::down(float d) {
    mutate([&](Camera& c) { c.down(d); });
}
void SceneCamera::pitch(float d) {
    mutate([&](Camera& c) { c.pitch(d); });
}
void SceneCamera::yaw(float d) {
    mutate([&](Camera& c) { c.yaw(d); });
}
void SceneCamera::roll(float d) {
    mutate([&](Camera& c) { c.roll(d); });
}

}  // namespace ICE
