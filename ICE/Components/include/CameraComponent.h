#pragma once

#include <memory>

#include "Component.h"

namespace ICE {
// Forward-declared, not included: keeps this component free of any graphics-backend dependency.
// `target` is only ever held here as a shared_ptr, never dereferenced (see below).
class Framebuffer;

// Per-entity camera. Attach it to an entity that also has a TransformComponent: the entity's world
// transform (position + orientation, and any scene-graph parent) becomes the view, and these
// parameters become the projection. A scene selects which camera entity is active
// (Scene::setActiveCamera) and the render system views the scene through it.
//
// Because the view is just the entity's world matrix, parenting the camera entity under another
// entity gives a follow camera with no special-case code, and switching the active camera entity
// switches the viewpoint.
struct CameraComponent : public Component {
    enum class Projection { Perspective, Orthographic };

    Projection projection = Projection::Perspective;

    // Perspective: vertical field of view in degrees. Matches the engine's historical default
    // camera (60 deg) so an entity camera renders the same view a scene-owned camera did.
    float fov = 60.0f;

    // Orthographic: half-height of the view volume in world units (width follows the aspect ratio).
    float ortho_size = 10.0f;

    float near_plane = 0.01f;
    float far_plane = 10000.0f;

    // Optional off-screen target for this camera (render-to-texture, split-screen). Reserved: the
    // render path does not consume it yet -- it renders the active camera to the scene target. Held
    // as a forward-declared shared_ptr so declaring it adds no graphics dependency to Components.
    std::shared_ptr<Framebuffer> target;
};
}  // namespace ICE
