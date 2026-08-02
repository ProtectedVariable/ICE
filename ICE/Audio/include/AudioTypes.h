#pragma once

#include <Asset.h>
#include <Eigen/Dense>
#include <HandlePool.h>

#include <cstdint>

namespace ICE {

// A playing (or paused) sound instance. Generational, so a handle to a one-shot that has since
// finished and had its slot recycled resolves to nothing rather than silently controlling whatever
// sound now occupies that slot. This matters more here than for GPU resources: one-shot voices are
// recycled constantly, and gameplay code routinely holds a handle across frames.
struct VoiceTag {};
using VoiceHandle = Handle<VoiceTag>;

// A backend-resident audio buffer uploaded from an AudioClip (an ALuint, for the OpenAL backend).
// Owned by AudioRegistry.
struct AudioBufferTag {};
using AudioBufferHandle = Handle<AudioBufferTag>;

enum class PlaybackState { Stopped, Playing, Paused };

// Mixer routing. The bus graph itself (per-bus gain, mute/solo) is phase 3; the id travels with
// voices from phase 1 so adding the graph later is additive rather than a signature change.
enum class BusId : uint8_t { Master = 0, Music = 1, SFX = 2, UI = 3, Voice = 4, Count = 5 };

// How gain falls off with distance. NOTE: OpenAL's distance model is a *global* context setting
// (alDistanceModel), not per-source -- per-source you only get reference/max distance and rolloff.
// The engine therefore treats this as a project-wide setting; see AudioDeviceConfig.
enum class AttenuationModel { None, InverseDistance, LinearDistance, ExponentDistance };

// Per-voice mutable state. Everything here can change every frame while a voice plays.
struct VoiceParams {
    Eigen::Vector3f position = Eigen::Vector3f::Zero();
    Eigen::Vector3f velocity = Eigen::Vector3f::Zero();  // for Doppler (phase 2)
    float gain = 1.0f;
    float pitch = 1.0f;
    bool looping = false;
    // False makes the voice head-relative at the origin, i.e. plain 2D playback at full volume --
    // the right mode for music and UI. True requires a mono clip (see AudioClip::isMono).
    bool spatial = false;
    float minDistance = 1.0f;   // AL_REFERENCE_DISTANCE: no attenuation closer than this
    float maxDistance = 500.0f;  // AL_MAX_DISTANCE
    float rolloff = 1.0f;        // AL_ROLLOFF_FACTOR
};

// What a voice is being acquired for. Immutable for the voice's lifetime.
struct VoiceDesc {
    AssetUID clip = NO_ASSET_ID;
    BusId bus = BusId::SFX;
    // Higher survives. When every source is in use, the pool steals the lowest-priority voice,
    // breaking ties by audibility (see AudioEngine::steal).
    uint8_t priority = 128;
    VoiceParams params;
};

struct ListenerState {
    Eigen::Vector3f position = Eigen::Vector3f::Zero();
    Eigen::Vector3f velocity = Eigen::Vector3f::Zero();
    Eigen::Vector3f forward = -Eigen::Vector3f::UnitZ();
    Eigen::Vector3f up = Eigen::Vector3f::UnitY();
    float gain = 1.0f;
};

struct AudioDeviceConfig {
    // Voices to request from the driver. OpenAL Soft's default context allocates 255 mono but only
    // ONE stereo source, which would cap non-spatialized playback (music + UI) at a single
    // simultaneous sound -- so both are requested explicitly at context creation.
    int monoVoices = 128;
    int stereoVoices = 16;
    bool preferHRTF = true;
    AttenuationModel attenuation = AttenuationModel::InverseDistance;
    float dopplerFactor = 1.0f;
    float speedOfSound = 343.3f;  // m/s
};

}  // namespace ICE
