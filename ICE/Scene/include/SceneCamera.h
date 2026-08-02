#pragma once

#include <Camera.h>
#include <Entity.h>

#include <memory>

namespace ICE {
class Registry;
class TransformComponent;

// A Camera whose pose can come from an entity (its CameraComponent + TransformComponent), so the
// engine's one "camera is a scene-owned object" exception becomes an ordinary component. This is
// what Scene::camera()/cameraPtr() hand out, so every existing consumer -- the render system's
// prepareFrame(Camera&), the editor -- keeps working unchanged.
//
// Two modes:
//   * Free (default, no bound entity): behaves exactly like the scene-owned PerspectiveCamera it
//     replaces -- every call delegates to an internal camera, so existing scenes are unaffected.
//   * Bound (Scene::setActiveCamera): the view is the bound entity's WORLD transform, so a camera
//     parented under a moving entity follows it with no special case; the projection comes from the
//     entity's CameraComponent.
class SceneCamera : public Camera {
   public:
    // Free camera matching the engine's historical scene default (60 deg fov, 16:9, 0.01..10000).
    SceneCamera();

    // Bind to a camera entity: its CameraComponent drives the projection, its world transform the
    // view. A null registry or an entity without a CameraComponent falls back to free behaviour.
    void bindEntity(Registry* registry, Entity entity);
    bool isBound() const { return m_registry != nullptr && m_entity != NULL_ENTITY; }
    Entity boundEntity() const { return m_entity; }

    Eigen::Matrix4f lookThrough() override;
    Eigen::Matrix4f getProjection() const override;
    Eigen::Vector3f getPosition() const override;
    void setPosition(const Eigen::Vector3f& p) override;
    Eigen::Vector3f getRotation() const override;
    void setRotation(const Eigen::Vector3f& r) override;

    void forward(float d) override;
    void backward(float d) override;
    void left(float d) override;
    void right(float d) override;
    void up(float d) override;
    void down(float d) override;
    void pitch(float d) override;
    void yaw(float d) override;
    void roll(float d) override;

    void resize(float width, float height) override;

   private:
    // The bound entity's transform, or nullptr in free mode / if it has none.
    TransformComponent* transform() const;

    // Rebuild m_impl (the projection source + free-mode camera) from the bound CameraComponent, or
    // to the free default when unbound. Preserves the current aspect ratio.
    void rebuildImpl();

    // Run a Camera mutation (a move/rotate). In free mode it just runs on m_impl. In bound mode it
    // loads the entity's local pose into m_impl, applies the op, and writes the result back -- so
    // every mutator reuses the existing camera math exactly, and edits land on the entity's
    // transform (composing with the scene graph like any other).
    template<typename Op>
    void mutate(Op&& op);

    // Free-mode camera and, when bound, the projection source (kept in sync with the
    // CameraComponent's params). Never null.
    std::shared_ptr<Camera> m_impl;
    float m_aspect = 16.0f / 9.0f;

    Registry* m_registry = nullptr;
    Entity m_entity = NULL_ENTITY;
};
}  // namespace ICE
