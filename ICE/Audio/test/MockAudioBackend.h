#pragma once

#include <IAudioBackend.h>

#include <algorithm>
#include <vector>

namespace ICE {

// Recording backend for tests: behaves like a real one (fixed capacity, generational handles that
// go stale on release) but records what it was told to do instead of making sound. This is what
// lets the voice-pool, stealing and eviction logic be tested exactly as it will run, with no audio
// device present -- which is the situation on CI.
class MockAudioBackend : public IAudioBackend {
   public:
    struct VoiceRecord {
        AudioBufferHandle buffer;
        VoiceDesc desc;
        VoiceParams params;
        PlaybackState state = PlaybackState::Stopped;
        // Set by the test to simulate a one-shot reaching its end; AudioEngine::update should then
        // reclaim the voice.
        bool finished = false;
    };

    explicit MockAudioBackend(std::size_t capacity = 4) : m_capacity(capacity) {}

    bool initialize(const AudioDeviceConfig& config) override {
        initialized = true;
        lastConfig = config;
        return initializeSucceeds;
    }
    void shutdown() override { initialized = false; }
    bool isAvailable() const override { return initialized; }
    std::string deviceName() const override { return "mock"; }

    AudioBufferHandle uploadClip(const AudioClip&) override {
        ++uploadCount;
        return m_buffers.insert(0);
    }
    void releaseBuffer(AudioBufferHandle buffer) override {
        if (m_buffers.erase(buffer)) {
            ++releaseBufferCount;
        }
    }

    VoiceHandle acquireVoice(AudioBufferHandle buffer, const VoiceDesc& desc) override {
        if (m_voices.size() >= m_capacity) {
            ++acquireFailures;
            return {};
        }
        return m_voices.insert(VoiceRecord{buffer, desc, desc.params, PlaybackState::Stopped, false});
    }
    void releaseVoice(VoiceHandle voice) override {
        if (m_voices.erase(voice)) {
            ++releaseVoiceCount;
        }
    }

    void setVoiceParams(VoiceHandle voice, const VoiceParams& params) override {
        if (auto* v = m_voices.get(voice)) {
            v->params = params;
        }
    }
    void setVoiceState(VoiceHandle voice, PlaybackState state) override {
        if (auto* v = m_voices.get(voice)) {
            v->state = state;
        }
    }
    bool isVoiceActive(VoiceHandle voice) const override {
        const auto* v = m_voices.get(voice);
        return v != nullptr && !v->finished && v->state != PlaybackState::Stopped;
    }

    std::size_t activeVoiceCount() const override { return m_voices.size(); }
    std::size_t voiceCapacity() const override { return m_capacity; }

    void setListener(const ListenerState& listener) override {
        lastListener = listener;
        ++setListenerCount;
    }
    void update(double delta) override {
        ++updateCount;
        lastDelta = delta;
    }

    // --- test helpers ---------------------------------------------------------------------------
    VoiceRecord* voice(VoiceHandle h) { return m_voices.get(h); }
    void finish(VoiceHandle h) {
        if (auto* v = m_voices.get(h)) {
            v->finished = true;
        }
    }
    std::vector<VoiceDesc> liveDescs() {
        std::vector<VoiceDesc> out;
        m_voices.forEachHandle([&](VoiceHandle, VoiceRecord& v) { out.push_back(v.desc); });
        return out;
    }
    std::size_t residentBuffers() const { return m_buffers.size(); }

    bool initialized = false;
    bool initializeSucceeds = true;
    AudioDeviceConfig lastConfig{};
    ListenerState lastListener{};
    double lastDelta = 0.0;
    int uploadCount = 0;
    int releaseBufferCount = 0;
    int releaseVoiceCount = 0;
    int acquireFailures = 0;
    int setListenerCount = 0;
    int updateCount = 0;

   private:
    HandlePool<VoiceRecord, VoiceTag> m_voices;
    HandlePool<int, AudioBufferTag> m_buffers;
    std::size_t m_capacity;
};

}  // namespace ICE
