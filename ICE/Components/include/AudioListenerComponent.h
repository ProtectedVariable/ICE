#pragma once

#include <Eigen/Dense>

#include "Component.h"

namespace ICE {

// Marks the entity whose world transform is the ear of the scene: its position, forward and up
// become the OpenAL listener, and every spatial source is mixed relative to it.
//
// Attaching this is OPTIONAL. By default AudioSystem uses the scene's active camera entity, which
// is what a single-viewpoint game wants and needs no setup. Add this component to decouple hearing
// from seeing -- a third-person game that should hear from the character rather than the orbiting
// camera, for example. If several entities carry one, the first active one found wins (and
// AudioSystem says so, since it is almost always a mistake).
struct AudioListenerComponent : public Component {
    AudioListenerComponent() = default;

    // Scales every sound this listener hears; the master volume control that survives scene loads.
    float volume = 1.0f;

    // Lets a listener be disabled without removing the component -- e.g. switching between two
    // authored viewpoints.
    bool active = true;

    // --- runtime only: never serialized -----------------------------------------------------
    // Previous frame's world position, for the listener's Doppler velocity.
    Eigen::Vector3f last_world_position = Eigen::Vector3f::Zero();
    bool has_last_position = false;
};

}  // namespace ICE
