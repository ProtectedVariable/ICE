#pragma once

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

    // Update a live voice (position, gain, pitch, ...). No-op for a stale handle.
    void setVoiceParams(VoiceHandle voice, const VoiceParams& params);
    // Null for a stale handle. Points at engine-owned storage; valid until the voice ends.
    const VoiceParams* getVoiceParams(VoiceHandle voice) const;

    // --- Global --------------------------------------------------------------------------------
    void setMasterGain(float gain);
    float getMasterGain() const { return m_master_gain; }
    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }

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
    struct ActiveVoice {
        VoiceHandle handle;
        VoiceDesc desc;
    };

    // Free the least valuable playing voice so a new one can start. Returns false when nothing is
    // a worse candidate than the incoming sound, in which case the new sound is simply dropped --
    // killing an *more* important sound to play a less important one is never right.
    bool steal(const VoiceDesc& incoming);

    // Distance-attenuated gain, used to rank voices for stealing. Mirrors the inverse-distance
    // model (the engine default); an exact match with the backend's curve is unnecessary, since
    // this only has to order voices sensibly.
    float audibility(const VoiceDesc& desc) const;

    // Gain actually pushed to the backend: the voice's own gain scaled by master gain and mute.
    float effectiveGain(float voiceGain) const;

    std::vector<ActiveVoice>::iterator find(VoiceHandle voice);
    std::vector<ActiveVoice>::const_iterator find(VoiceHandle voice) const;

    std::shared_ptr<IAudioBackend> m_backend;
    std::unique_ptr<AudioRegistry> m_registry;
    std::vector<ActiveVoice> m_active;
    ListenerState m_listener;
    float m_master_gain = 1.0f;
    bool m_muted = false;
    std::size_t m_stolen_count = 0;
    // Clips already reported as un-spatializable (stereo), so the warning fires once per clip
    // rather than on every play call.
    std::unordered_set<AssetUID> m_warned_stereo;
};

}  // namespace ICE
