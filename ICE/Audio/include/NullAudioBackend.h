#pragma once

#include "IAudioBackend.h"

namespace ICE {

// Silent reference backend, mirroring NullPhysicsBackend. Two jobs:
//   * the fallback when no audio device can be opened, so a headless machine (CI, a server build)
//     runs the full engine with audio "attached" but inert instead of failing;
//   * the template a real backend is written against.
//
// It honours the voice-accounting contract -- a fixed capacity, handles that go stale on release --
// so code paths that depend on acquisition failing under load behave identically here.
class NullAudioBackend : public IAudioBackend {
   public:
    explicit NullAudioBackend(std::size_t capacity = 32) : m_capacity(capacity) {}

    bool initialize(const AudioDeviceConfig&) override { return true; }
    void shutdown() override {
        m_voices = {};
        m_buffers = {};
    }

    bool isAvailable() const override { return false; }  // audible output: none
    std::string deviceName() const override { return "null"; }

    AudioBufferHandle uploadClip(const AudioClip&) override { return m_buffers.insert(0); }
    void releaseBuffer(AudioBufferHandle buffer) override { m_buffers.erase(buffer); }

    VoiceHandle acquireVoice(AudioBufferHandle, const VoiceDesc&) override {
        if (m_voices.size() >= m_capacity) {
            return {};
        }
        return m_voices.insert(PlaybackState::Playing);
    }
    // Streaming behaves exactly like a resident voice here: silent either way, but it keeps the
    // voice accounting identical so code paths that depend on acquisition failing under load
    // behave the same on the null backend.
    VoiceHandle acquireStreamingVoice(const std::shared_ptr<IAudioStream>&, const VoiceDesc&) override {
        if (m_voices.size() >= m_capacity) {
            return {};
        }
        return m_voices.insert(PlaybackState::Playing);
    }
    void releaseVoice(VoiceHandle voice) override { m_voices.erase(voice); }

    void setVoiceParams(VoiceHandle, const VoiceParams&) override {}
    void setVoiceState(VoiceHandle voice, PlaybackState state) override {
        if (auto* s = m_voices.get(voice)) {
            *s = state;
        }
    }
    // A null voice never finishes on its own -- there is no clock driving it. Callers stop it
    // explicitly, which keeps the silent path deterministic for tests.
    bool isVoiceActive(VoiceHandle voice) const override {
        const auto* s = m_voices.get(voice);
        return s != nullptr && *s != PlaybackState::Stopped;
    }

    std::size_t activeVoiceCount() const override { return m_voices.size(); }
    std::size_t voiceCapacity() const override { return m_capacity; }

    void setListener(const ListenerState&) override {}
    void update(double) override {}

   private:
    HandlePool<PlaybackState, VoiceTag> m_voices;
    HandlePool<int, AudioBufferTag> m_buffers;
    std::size_t m_capacity;
};

}  // namespace ICE
