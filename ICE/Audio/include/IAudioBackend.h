#pragma once

#include <memory>
#include <string>

#include "AudioTypes.h"

namespace ICE {
class AudioClip;
class IAudioStream;
class JobScheduler;

// Engine-level audio seam, mirroring RendererAPI/IPhysicsBackend: the engine drives an
// IAudioBackend without knowing whether the mixer underneath is OpenAL, a null stub, or a test
// mock. A new backend plugs in via ICEEngine::setAudioBackend with no change to core.
//
// Division of responsibility: the backend is MECHANISM (open the device, own the sources, push
// parameters), AudioEngine is POLICY (which voice to steal, bus gains, one-shot bookkeeping). A
// backend never decides that a sound is unimportant; it just reports that it is out of voices.
class IAudioBackend {
   public:
    virtual ~IAudioBackend() = default;

    // Returns false if no device could be opened -- the expected outcome on headless CI. The
    // engine then falls back to NullAudioBackend rather than treating audio as fatal.
    virtual bool initialize(const AudioDeviceConfig& config) = 0;
    virtual void shutdown() = 0;

    virtual bool isAvailable() const = 0;
    virtual std::string deviceName() const = 0;

    // --- Buffers (owned by AudioRegistry) -------------------------------------------------------
    // Upload a decoded clip. Returns a null handle if the clip is empty or the upload fails.
    virtual AudioBufferHandle uploadClip(const AudioClip& clip) = 0;
    virtual void releaseBuffer(AudioBufferHandle buffer) = 0;

    // --- Voices ---------------------------------------------------------------------------------
    // Start `buffer` playing. Returns a null handle when no source is free; the caller (AudioEngine)
    // decides whether to steal and retry. Never blocks, never allocates a device object -- the
    // source set is fixed at initialize().
    virtual VoiceHandle acquireVoice(AudioBufferHandle buffer, const VoiceDesc& desc) = 0;

    // Start a voice fed incrementally from `stream` rather than from a resident buffer. Used for
    // music and other long sounds. Looping is driven by rewinding the stream (desc.params.looping),
    // because AL_LOOPING on a queued-buffer source would loop one queued chunk, not the sound.
    //
    // Backends that cannot stream return a null handle; the caller falls back to silence rather
    // than to a multi-megabyte resident decode.
    virtual VoiceHandle acquireStreamingVoice(const std::shared_ptr<IAudioStream>& stream, const VoiceDesc& desc) = 0;

    virtual void releaseVoice(VoiceHandle voice) = 0;

    virtual void setVoiceParams(VoiceHandle voice, const VoiceParams& params) = 0;
    virtual void setVoiceState(VoiceHandle voice, PlaybackState state) = 0;
    // False once the sound has run to its end (or the handle is stale). This is what AudioEngine
    // polls in update() to reclaim finished one-shots.
    virtual bool isVoiceActive(VoiceHandle voice) const = 0;

    // Number of voices currently in use / the hard ceiling imposed by the device.
    virtual std::size_t activeVoiceCount() const = 0;
    virtual std::size_t voiceCapacity() const = 0;

    virtual void setListener(const ListenerState& listener) = 0;

    // Per-frame bookkeeping. Mixing happens on the backend's own thread; this is main-thread
    // housekeeping only (reclaiming stopped sources, refilling streaming buffer queues).
    virtual void update(double delta) = 0;

    // Optional scheduler used to decode streaming audio off the main thread. Without one, streams
    // decode inline in update() -- correct, but a multi-millisecond spike on the frame that
    // happens to need a refill. Null detaches it.
    virtual void setScheduler(const std::shared_ptr<JobScheduler>& /*scheduler*/) {}

    // Diagnostics: how many streaming voices have starved (run out of decoded audio while still
    // playing). Non-zero means decoding is not keeping up -- the number worth watching when tuning
    // chunk size or count.
    virtual std::size_t streamUnderrunCount() const { return 0; }

    // --- Phase 5 seams --------------------------------------------------------------------------
    // Defaulted to no-ops, following GraphicsFactory::createTexture2D: declaring them now means
    // reverb and occlusion plug in without a later signature change rippling through the stack.
    virtual void setReverbPreset(int /*preset*/) {}
    virtual void setVoiceOcclusion(VoiceHandle /*voice*/, float /*occlusion01*/) {}
};

}  // namespace ICE
