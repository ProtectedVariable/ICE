#pragma once

#include <array>
#include <memory>
#include <unordered_set>
#include <vector>

#include "AudioRegistry.h"
#include "IAudioBackend.h"

namespace ICE {

// The common knobs for a fire-and-forget sound, so callers don't have to fill a whole VoiceDesc:
//   engine.audio()->play(clipId, {.volume = 0.5f, .loop = true});
struct PlayParams {
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    BusId bus = BusId::SFX;
    uint8_t priority = 128;
    // Ramp from silence up to `volume` over this many seconds. 0 starts at full volume.
    float fadeInSeconds = 0.0f;
};

// The engine's audio facade and the owner of voice POLICY. The backend reports that it is out of
// sources; deciding which existing sound dies so a new one can play happens here, in one place,
// independent of which backend is attached.
//
// Everything on this class is main-thread only. The backend mixes on its own thread, but no call
// here touches that thread's state directly.
class AudioEngine {
   public:
    AudioEngine(const std::shared_ptr<IAudioBackend>& backend, const std::shared_ptr<AssetBank>& bank);
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    // --- Playback -------------------------------------------------------------------------------
    // 2D (head-relative, unattenuated) playback -- music, UI, narration. Works with stereo clips.
    VoiceHandle play(AssetUID clip, const PlayParams& params = {});

    // Positional playback. The clip MUST be mono: OpenAL silently refuses to spatialize a stereo
    // buffer and plays it flat at full volume instead. A stereo clip here is logged and played 2D
    // rather than pretending to be positioned.
    VoiceHandle playAt(AssetUID clip, const Eigen::Vector3f& position, const PlayParams& params = {});

    // Full control, for callers that build their own VoiceDesc (the phase 2 AudioSystem).
    VoiceHandle playVoice(const VoiceDesc& desc);

    void stop(VoiceHandle voice);
    void stopAll();
    void pause(VoiceHandle voice);
    void resume(VoiceHandle voice);
    bool isPlaying(VoiceHandle voice) const;

    // --- Fades ----------------------------------------------------------------------------------
    // Ramp a live voice's gain over `seconds`. Gains here are the voice's AUTHORED gain (the same
    // scale as PlayParams::volume); bus and master scaling are applied on top as usual, so a fade
    // and a mixer change compose instead of fighting.
    //
    // Ramps are advanced in update(), so they cost nothing until they are used and are unaffected
    // by how the backend mixes.
    void fadeTo(VoiceHandle voice, float targetGain, float seconds);
    void fadeIn(VoiceHandle voice, float targetGain, float seconds);
    // Ramp to silence and then STOP the voice, releasing it. This is the one that matters for
    // music: stopping outright produces an audible click.
    void fadeOut(VoiceHandle voice, float seconds);

    // Fade `current` out while starting `clip` faded in over the same interval, and return the new
    // voice. The outgoing voice is released when its ramp completes. Passing an invalid `current`
    // just starts the new sound, so this works for the first track too.
    VoiceHandle crossfadeTo(VoiceHandle current, AssetUID clip, float seconds, const PlayParams& params = {});

    bool isFading(VoiceHandle voice) const;

    // Update a live voice (position, gain, pitch, ...). No-op for a stale handle.
    void setVoiceParams(VoiceHandle voice, const VoiceParams& params);
    // Null for a stale handle. Points at engine-owned storage; valid until the voice ends.
    const VoiceParams* getVoiceParams(VoiceHandle voice) const;

    // --- Global --------------------------------------------------------------------------------
    void setMasterGain(float gain);
    float getMasterGain() const { return m_master_gain; }
    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }

    // Silence everything without touching the user's mute setting -- used for "pause audio while
    // the window is in the background". Kept separate from setMuted precisely so that un-focusing
    // and re-focusing cannot clobber a mute the user chose, and vice versa.
    void setSuspended(bool suspended);
    bool isSuspended() const { return m_suspended; }

    // --- Mixer buses ---------------------------------------------------------------------------
    // A voice's audible gain is (its own gain) x (its bus gain) x (master gain), zeroed if the bus
    // or the master is muted. OpenAL has no native submix, so buses are applied as a gain
    // multiplier on the way to the device rather than as a real graph -- inaudible difference for
    // level control, and it keeps the backend seam free of mixer concepts.
    //
    // Changing a bus re-pushes only the voices routed to it; per-voice gains are the source of
    // truth, so repeated calls never compound.
    void setBusGain(BusId bus, float gain);
    float getBusGain(BusId bus) const;
    void setBusMuted(BusId bus, bool muted);
    bool isBusMuted(BusId bus) const;

    void setListener(const ListenerState& listener);
    const ListenerState& getListener() const { return m_listener; }

    // Per-frame: drives backend housekeeping and reclaims voices that have run to their end.
    void update(double delta);

    AudioRegistry& getRegistry() { return *m_registry; }
    IAudioBackend& getBackend() { return *m_backend; }

    std::size_t getActiveVoiceCount() const { return m_active.size(); }
    // Cumulative count of voices killed to make room. A steadily climbing number means the pool is
    // undersized for the scene -- worth surfacing in the editor's audio panel.
    std::size_t getStolenVoiceCount() const { return m_stolen_count; }

   private:
    // A linear gain ramp on a voice's authored gain. `active` false means no ramp is running.
    struct Fade {
        bool active = false;
        float from = 0.0f;
        float to = 0.0f;
        float elapsed = 0.0f;
        float duration = 0.0f;
        // Release the voice once the ramp lands (fade-out / the outgoing half of a crossfade).
        bool stopAtEnd = false;
    };

    struct ActiveVoice {
        VoiceHandle handle;
        VoiceDesc desc;
        Fade fade;
    };

    // Advance every running ramp by `delta` and push the resulting gains. Voices whose fade-out
    // completed are stopped here.
    void advanceFades(double delta);

    // Free the least valuable playing voice so a new one can start. Returns false when nothing is
    // a worse candidate than the incoming sound, in which case the new sound is simply dropped --
    // killing an *more* important sound to play a less important one is never right.
    bool steal(const VoiceDesc& incoming);

    // Distance-attenuated gain, used to rank voices for stealing. Mirrors the inverse-distance
    // model (the engine default); an exact match with the backend's curve is unnecessary, since
    // this only has to order voices sensibly.
    float audibility(const VoiceDesc& desc) const;

    // Gain actually pushed to the backend: the voice's own gain scaled by its bus and the master,
    // with either mute forcing silence.
    float effectiveGain(const VoiceDesc& desc) const;

    // Re-push the device gain for one live voice from its stored authored gain.
    void repushGain(const ActiveVoice& voice);

    static constexpr std::size_t kBusCount = static_cast<std::size_t>(BusId::Count);
    static std::size_t busIndex(BusId bus) {
        auto i = static_cast<std::size_t>(bus);
        return i < kBusCount ? i : static_cast<std::size_t>(BusId::SFX);
    }

    std::vector<ActiveVoice>::iterator find(VoiceHandle voice);
    std::vector<ActiveVoice>::const_iterator find(VoiceHandle voice) const;

    std::shared_ptr<IAudioBackend> m_backend;
    std::unique_ptr<AudioRegistry> m_registry;
    std::vector<ActiveVoice> m_active;
    ListenerState m_listener;
    float m_master_gain = 1.0f;
    bool m_muted = false;
    bool m_suspended = false;
    std::array<float, kBusCount> m_bus_gain{};
    std::array<bool, kBusCount> m_bus_muted{};
    std::size_t m_stolen_count = 0;
    // Clips already reported as un-spatializable (stereo), so the warning fires once per clip
    // rather than on every play call.
    std::unordered_set<AssetUID> m_warned_stereo;
};

}  // namespace ICE
