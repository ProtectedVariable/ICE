#include "AudioEngine.h"

#include <AudioClip.h>
#include <Logger.h>

#include <algorithm>

namespace ICE {

AudioEngine::AudioEngine(const std::shared_ptr<IAudioBackend>& backend, const std::shared_ptr<AssetBank>& bank)
    : m_backend(backend),
      m_registry(std::make_unique<AudioRegistry>(backend, bank)) {}

AudioEngine::~AudioEngine() {
    stopAll();
    // Buffers must go before the backend does, while it can still release them.
    m_registry.reset();
}

VoiceHandle AudioEngine::play(AssetUID clip, const PlayParams& params) {
    VoiceDesc desc;
    desc.clip = clip;
    desc.bus = params.bus;
    desc.priority = params.priority;
    desc.params.gain = params.volume;
    desc.params.pitch = params.pitch;
    desc.params.looping = params.loop;
    desc.params.spatial = false;
    return playVoice(desc);
}

VoiceHandle AudioEngine::playAt(AssetUID clip, const Eigen::Vector3f& position, const PlayParams& params) {
    VoiceDesc desc;
    desc.clip = clip;
    desc.bus = params.bus;
    desc.priority = params.priority;
    desc.params.gain = params.volume;
    desc.params.pitch = params.pitch;
    desc.params.looping = params.loop;
    desc.params.spatial = true;
    desc.params.position = position;
    return playVoice(desc);
}

VoiceHandle AudioEngine::playVoice(const VoiceDesc& desc) {
    if (m_backend == nullptr || desc.clip == NO_ASSET_ID) {
        return {};
    }

    AudioBufferHandle buffer = m_registry->getBuffer(desc.clip);
    if (!buffer.valid()) {
        return {};  // unknown, still loading, or failed to decode -- silent, not fatal
    }

    VoiceDesc effective = desc;

    // Spatialization silently does nothing for a stereo buffer in OpenAL: it plays flat at full
    // volume, ignoring position entirely. Rather than let that look like a positioning bug, demote
    // it to explicit 2D playback and say so -- once per clip, since this is usually an import
    // mistake that would otherwise spam a per-frame log.
    if (effective.params.spatial) {
        auto clip = m_registry->getClip(desc.clip);
        if (clip != nullptr && !clip->isMono()) {
            if (m_warned_stereo.insert(desc.clip).second) {
                Logger::Log(Logger::WARNING, "Audio",
                            "Clip %llu is %u-channel and cannot be spatialized (OpenAL positions mono buffers only); "
                            "playing it 2D. Re-import it as mono if it should be positional.",
                            (unsigned long long) desc.clip, clip->getChannels());
            }
            effective.params.spatial = false;
        }
    }

    VoiceHandle voice = m_backend->acquireVoice(buffer, effective);
    if (!voice.valid()) {
        if (!steal(effective)) {
            return {};  // everything playing is more important than this
        }
        voice = m_backend->acquireVoice(buffer, effective);
        if (!voice.valid()) {
            return {};
        }
    }

    // Apply master gain / mute on the way to the device; m_active keeps the voice's own gain so a
    // later master-gain change can be recomputed from it.
    VoiceParams device_params = effective.params;
    device_params.gain = effectiveGain(effective.params.gain);
    m_backend->setVoiceParams(voice, device_params);
    m_backend->setVoiceState(voice, PlaybackState::Playing);

    m_active.push_back({voice, effective});
    return voice;
}

void AudioEngine::stop(VoiceHandle voice) {
    auto it = find(voice);
    if (it == m_active.end()) {
        return;
    }
    m_backend->setVoiceState(voice, PlaybackState::Stopped);
    m_backend->releaseVoice(voice);
    m_active.erase(it);
}

void AudioEngine::stopAll() {
    if (m_backend == nullptr) {
        return;
    }
    for (const auto& v : m_active) {
        m_backend->setVoiceState(v.handle, PlaybackState::Stopped);
        m_backend->releaseVoice(v.handle);
    }
    m_active.clear();
}

void AudioEngine::pause(VoiceHandle voice) {
    if (find(voice) != m_active.end()) {
        m_backend->setVoiceState(voice, PlaybackState::Paused);
    }
}

void AudioEngine::resume(VoiceHandle voice) {
    if (find(voice) != m_active.end()) {
        m_backend->setVoiceState(voice, PlaybackState::Playing);
    }
}

bool AudioEngine::isPlaying(VoiceHandle voice) const {
    return find(voice) != m_active.end() && m_backend->isVoiceActive(voice);
}

void AudioEngine::setVoiceParams(VoiceHandle voice, const VoiceParams& params) {
    auto it = find(voice);
    if (it == m_active.end()) {
        return;
    }
    it->desc.params = params;
    VoiceParams device_params = params;
    device_params.gain = effectiveGain(params.gain);
    m_backend->setVoiceParams(voice, device_params);
}

const VoiceParams* AudioEngine::getVoiceParams(VoiceHandle voice) const {
    auto it = find(voice);
    return it == m_active.end() ? nullptr : &it->desc.params;
}

void AudioEngine::setMasterGain(float gain) {
    m_master_gain = std::clamp(gain, 0.0f, 1.0f);
    // Re-push every live voice's gain: the stored per-voice gain is the source of truth, so this
    // is idempotent and never compounds.
    for (const auto& v : m_active) {
        VoiceParams device_params = v.desc.params;
        device_params.gain = effectiveGain(v.desc.params.gain);
        m_backend->setVoiceParams(v.handle, device_params);
    }
}

void AudioEngine::setMuted(bool muted) {
    if (m_muted == muted) {
        return;
    }
    m_muted = muted;
    setMasterGain(m_master_gain);  // re-push through the same path
}

void AudioEngine::setListener(const ListenerState& listener) {
    m_listener = listener;
    if (m_backend != nullptr) {
        m_backend->setListener(listener);
    }
}

void AudioEngine::update(double delta) {
    if (m_backend == nullptr) {
        return;
    }
    m_backend->update(delta);

    // Reclaim one-shots that have run to their end. Looping voices stay active until stopped.
    std::erase_if(m_active, [this](const ActiveVoice& v) {
        if (m_backend->isVoiceActive(v.handle)) {
            return false;
        }
        m_backend->releaseVoice(v.handle);
        return true;
    });
}

bool AudioEngine::steal(const VoiceDesc& incoming) {
    if (m_active.empty()) {
        return false;
    }

    const float incoming_audibility = audibility(incoming);
    auto victim = m_active.end();
    for (auto it = m_active.begin(); it != m_active.end(); ++it) {
        if (victim == m_active.end()) {
            victim = it;
            continue;
        }
        // Lowest priority first; ties broken by which is quieter at the listener.
        if (it->desc.priority < victim->desc.priority ||
            (it->desc.priority == victim->desc.priority && audibility(it->desc) < audibility(victim->desc))) {
            victim = it;
        }
    }

    // Never kill something more important than what is arriving.
    if (victim->desc.priority > incoming.priority ||
        (victim->desc.priority == incoming.priority && audibility(victim->desc) >= incoming_audibility)) {
        return false;
    }

    m_backend->setVoiceState(victim->handle, PlaybackState::Stopped);
    m_backend->releaseVoice(victim->handle);
    m_active.erase(victim);
    ++m_stolen_count;
    return true;
}

float AudioEngine::audibility(const VoiceDesc& desc) const {
    const float gain = desc.params.gain;
    if (!desc.params.spatial) {
        return gain;  // 2D sounds play at full volume wherever the listener is
    }
    const float distance = (desc.params.position - m_listener.position).norm();
    if (distance <= desc.params.minDistance) {
        return gain;
    }
    if (distance >= desc.params.maxDistance) {
        return 0.0f;
    }
    // Inverse-distance rolloff, matching the engine's default attenuation model. This only has to
    // ORDER voices sensibly, so an exact match with the backend's curve is not required.
    const float denom = desc.params.minDistance + desc.params.rolloff * (distance - desc.params.minDistance);
    return denom <= 0.0f ? gain : gain * (desc.params.minDistance / denom);
}

float AudioEngine::effectiveGain(float voiceGain) const {
    return m_muted ? 0.0f : voiceGain * m_master_gain;
}

std::vector<AudioEngine::ActiveVoice>::iterator AudioEngine::find(VoiceHandle voice) {
    return std::find_if(m_active.begin(), m_active.end(), [voice](const ActiveVoice& v) { return v.handle == voice; });
}

std::vector<AudioEngine::ActiveVoice>::const_iterator AudioEngine::find(VoiceHandle voice) const {
    return std::find_if(m_active.begin(), m_active.end(), [voice](const ActiveVoice& v) { return v.handle == voice; });
}

}  // namespace ICE
