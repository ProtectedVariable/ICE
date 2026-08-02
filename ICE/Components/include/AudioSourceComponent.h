#pragma once

#include <Asset.h>
#include <Eigen/Dense>

#include <cstdint>

#include "Component.h"

namespace ICE {

// Requested playback state for a source. This is the AUTHORED intent; whether a voice is actually
// sounding is a runtime question answered by AudioSystem (a source can be Playing while inaudible
// because its clip is still loading or its voice was stolen).
enum class AudioSourceState { Stopped, Playing, Paused };

// Attach alongside a TransformComponent to make an entity emit sound. The entity's world transform
// (resolved by SceneGraphSystem) supplies the position, so parenting a source under a moving entity
// makes it travel with no special-case code -- exactly as a camera entity gives a follow camera.
//
// Mirrors RenderComponent: it names an asset by UID and holds only plain data, so it stays free of
// any dependency on the audio backend.
struct AudioSourceComponent : public Component {
    AudioSourceComponent() = default;
    explicit AudioSourceComponent(AssetUID clip_id) : clip(clip_id) {}

    AssetUID clip = NO_ASSET_ID;

    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;

    // Start on the first frame the source is seen by AudioSystem. The usual way to author ambient
    // loops and music without any gameplay code.
    bool playOnAwake = false;

    // Positional playback. Requires a MONO clip -- OpenAL does not spatialize stereo buffers, it
    // plays them flat at full volume. AudioSystem reports a clip that violates this rather than
    // letting it look like a positioning bug.
    bool spatial = true;

    // Distance model parameters. No attenuation within minDistance; silent beyond maxDistance.
    float minDistance = 1.0f;
    float maxDistance = 500.0f;
    float rolloff = 1.0f;

    // Higher survives when the voice pool is exhausted (see AudioEngine's stealing policy).
    uint8_t priority = 128;

    // Mixer routing. Stored as a plain integer so this header stays independent of the audio
    // module's BusId enum; AudioSystem maps it across.
    uint8_t bus = 2;  // BusId::SFX

    AudioSourceState state = AudioSourceState::Stopped;

    // --- runtime only: never serialized -----------------------------------------------------
    // Index+generation of the live voice, as an opaque pair so this header does not depend on the
    // audio module's VoiceHandle. Zero generation means "no voice".
    uint32_t voice_index = 0;
    uint32_t voice_generation = 0;
    // Previous frame's world position, for the Doppler velocity estimate. Invalid until the source
    // has been seen once (tracked by has_last_position).
    Eigen::Vector3f last_world_position = Eigen::Vector3f::Zero();
    bool has_last_position = false;
    // Set once playOnAwake has been honoured, so it fires exactly once rather than restarting the
    // sound every time the source stops.
    bool awake_handled = false;
    // True once a voice has actually been created for the CURRENT play request. This is what
    // distinguishes "has not started yet" (keep trying -- the clip may still be decoding) from
    // "has already finished" (do not restart). Without it a one-shot would loop forever: the voice
    // ends, AudioEngine reclaims it, and the source sees state == Playing with no voice again.
    bool voice_started = false;
    // Previous frame's state, so AudioSystem can detect a fresh play request even when gameplay
    // assigns `state` directly instead of calling play().
    AudioSourceState last_state = AudioSourceState::Stopped;

    // Convenience for gameplay code: request playback from the next AudioSystem update. Assigning
    // `state` directly works too -- AudioSystem detects the transition either way.
    void play() { state = AudioSourceState::Playing; }
    void stop() { state = AudioSourceState::Stopped; }
    void pause() { state = AudioSourceState::Paused; }
};

}  // namespace ICE
