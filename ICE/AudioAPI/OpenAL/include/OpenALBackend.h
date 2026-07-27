#pragma once

#include <IAudioBackend.h>

#include <memory>

#include "OpenALDevice.h"

namespace ICE {

// IAudioBackend on OpenAL Soft.
//
// Threading: OpenAL Soft mixes on its own internal thread, and while its al* entry points are
// thread-safe in practice, the OpenAL spec makes no such guarantee and the calls take locks. Every
// method here is therefore MAIN-THREAD ONLY, driven by AudioEngine. (The phase 4 streaming refill
// thread will be the single, explicit exception, touching only its own sources.)
//
// The source set is allocated once in initialize() and never grows: alGenSources can fail
// mid-frame, and there is no graceful recovery from that during gameplay. Running out of voices is
// reported by acquireVoice returning a null handle, which AudioEngine answers with its stealing
// policy.
class OpenALBackend : public IAudioBackend {
   public:
    OpenALBackend();
    ~OpenALBackend() override;

    bool initialize(const AudioDeviceConfig& config) override;
    void shutdown() override;

    bool isAvailable() const override;
    std::string deviceName() const override;

    AudioBufferHandle uploadClip(const AudioClip& clip) override;
    void releaseBuffer(AudioBufferHandle buffer) override;

    VoiceHandle acquireVoice(AudioBufferHandle buffer, const VoiceDesc& desc) override;
    void releaseVoice(VoiceHandle voice) override;

    void setVoiceParams(VoiceHandle voice, const VoiceParams& params) override;
    void setVoiceState(VoiceHandle voice, PlaybackState state) override;
    bool isVoiceActive(VoiceHandle voice) const override;

    std::size_t activeVoiceCount() const override;
    std::size_t voiceCapacity() const override;

    void setListener(const ListenerState& listener) override;
    void update(double delta) override;

    // Device capabilities, for logging and the editor's audio panel.
    const OpenALDeviceInfo& deviceInfo() const;

   private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace ICE
