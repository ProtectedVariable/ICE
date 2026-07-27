#include "AudioSystem.h"

#include <Logger.h>

namespace ICE {

AudioSystem::AudioSystem(const std::shared_ptr<Registry>& registry, AudioEngine* audio)
    : m_registry(registry.get()),
      m_audio(audio) {}

void AudioSystem::update(double delta) {
    if (m_audio == nullptr || m_registry == nullptr) {
        return;
    }

    // Listener first: source gains and stealing decisions are ranked by distance to it, so a stale
    // listener would mis-rank everything this frame.
    m_active_listener = updateListener(delta);

    m_registry->each<AudioSourceComponent, TransformComponent>(
        [&](Entity e, AudioSourceComponent& source, TransformComponent& transform) { updateSource(e, source, transform, delta); });
}

Entity AudioSystem::updateListener(double delta) {
    Entity chosen = NULL_ENTITY;
    AudioListenerComponent* chosen_component = nullptr;
    int active_count = 0;

    // An explicit AudioListenerComponent always beats the camera fallback: it is the author saying
    // "hear from here, not from the camera".
    m_registry->each<AudioListenerComponent, TransformComponent>([&](Entity e, AudioListenerComponent& listener, TransformComponent&) {
        if (!listener.active) {
            return;
        }
        ++active_count;
        if (chosen == NULL_ENTITY) {
            chosen = e;
            chosen_component = &listener;
        }
    });

    if (active_count > 1 && !m_warned_multiple_listeners) {
        m_warned_multiple_listeners = true;
        Logger::Log(Logger::WARNING, "Audio", "%d active AudioListenerComponents in the scene; using entity %u. A scene has one ear.",
                    active_count, chosen);
    }

    if (chosen == NULL_ENTITY) {
        chosen = m_fallback_listener;  // the scene's active camera, supplied by the engine
    }
    if (chosen == NULL_ENTITY || !m_registry->isAlive(chosen)) {
        return NULL_ENTITY;
    }

    auto* transform = m_registry->tryGetComponent<TransformComponent>(chosen);
    if (transform == nullptr) {
        return NULL_ENTITY;  // a camera with no transform has no pose to hear from
    }

    const Eigen::Matrix4f world = transform->getWorldMatrix();
    ListenerState state;
    state.position = world.block<3, 1>(0, 3);
    // OpenAL's AL_ORIENTATION is (at, up). The engine's convention matches the camera's: -Z is
    // forward, +Y is up, so those come straight off the rotation columns of the world matrix.
    state.forward = -world.block<3, 1>(0, 2).normalized();
    state.up = world.block<3, 1>(0, 1).normalized();
    state.gain = chosen_component != nullptr ? chosen_component->volume : 1.0f;

    if (chosen_component != nullptr) {
        state.velocity =
            velocityFrom(state.position, chosen_component->last_world_position, chosen_component->has_last_position, delta);
    } else {
        // Camera-fallback listener: there is no component to cache the previous position in, so
        // keep it on the system itself.
        state.velocity = velocityFrom(state.position, m_fallback_last_position, m_fallback_has_last_position, delta);
    }

    m_audio->setListener(state);
    return chosen;
}

void AudioSystem::updateSource(Entity e, AudioSourceComponent& source, TransformComponent& transform, double delta) {
    const Eigen::Vector3f world_position = transform.getWorldMatrix().block<3, 1>(0, 3);
    const Eigen::Vector3f velocity = velocityFrom(world_position, source.last_world_position, source.has_last_position, delta);

    // playOnAwake fires exactly once, the first time the source is seen -- not every time it stops,
    // which would make a one-shot ambient sound restart forever.
    if (source.playOnAwake && !source.awake_handled) {
        source.awake_handled = true;
        source.state = AudioSourceState::Playing;
    }

    // A Stopped/Paused -> Playing transition is a FRESH play request, so the "already ran to
    // completion" latch resets. Detected from the state delta rather than from play() being called,
    // so assigning `state` directly behaves identically.
    const bool state_changed = source.state != source.last_state;
    if (state_changed && source.state == AudioSourceState::Playing) {
        source.voice_started = false;
    }
    source.last_state = source.state;

    const VoiceHandle voice = handleOf(source);
    const bool has_voice = m_audio->getVoiceParams(voice) != nullptr;

    switch (source.state) {
        case AudioSourceState::Stopped:
            if (has_voice) {
                m_audio->stop(voice);
            }
            clearHandle(source);
            source.voice_started = false;
            break;

        case AudioSourceState::Paused:
            if (has_voice && state_changed) {
                m_audio->pause(voice);
            }
            break;

        case AudioSourceState::Playing:
            if (has_voice) {
                if (state_changed) {
                    m_audio->resume(voice);  // covers Paused -> Playing
                }
                m_audio->setVoiceParams(voice, buildParams(source, world_position, velocity));
            } else if (source.voice_started) {
                // A voice existed and is now gone: the sound ran to its end (or was stolen). Settle
                // into Stopped instead of restarting -- without this latch a one-shot would replay
                // every frame forever.
                source.state = AudioSourceState::Stopped;
                source.last_state = AudioSourceState::Stopped;
                source.voice_started = false;
                clearHandle(source);
            } else {
                // Never started yet. A failure here is not final: the clip may still be decoding
                // (async import) or the pool may be momentarily full, so we retry next frame.
                startVoice(e, source, world_position);
            }
            break;
    }
}

void AudioSystem::startVoice(Entity e, AudioSourceComponent& source, const Eigen::Vector3f& world_position) {
    if (source.clip == NO_ASSET_ID) {
        return;
    }

    VoiceDesc desc;
    desc.clip = source.clip;
    desc.priority = source.priority;
    desc.bus = static_cast<BusId>(source.bus < static_cast<uint8_t>(BusId::Count) ? source.bus : static_cast<uint8_t>(BusId::SFX));
    desc.params = buildParams(source, world_position, Eigen::Vector3f::Zero());

    VoiceHandle voice = m_audio->playVoice(desc);
    if (!voice.valid()) {
        // Out of voices, or the clip is still decoding. voice_started stays false so the source
        // retries next frame -- which is exactly what an async import needs: it becomes audible the
        // moment its clip lands.
        clearHandle(source);
        return;
    }
    setHandle(source, voice);
    source.voice_started = true;
}

VoiceParams AudioSystem::buildParams(const AudioSourceComponent& source, const Eigen::Vector3f& world_position,
                                     const Eigen::Vector3f& velocity) const {
    VoiceParams params;
    params.position = world_position;
    params.velocity = velocity;
    params.gain = source.volume;
    params.pitch = source.pitch;
    params.looping = source.loop;
    params.spatial = source.spatial;
    params.minDistance = source.minDistance;
    params.maxDistance = source.maxDistance;
    params.rolloff = source.rolloff;
    return params;
}

void AudioSystem::onEntityRemoved(Entity e) {
    if (m_audio == nullptr || m_registry == nullptr) {
        return;
    }
    // The component may already be gone (that is often why the entity stopped matching), so probe
    // rather than assume. When it is gone, its voice is reclaimed by AudioEngine::update once the
    // sound ends -- handles are generational, so nothing dangles either way.
    if (auto* source = m_registry->tryGetComponent<AudioSourceComponent>(e)) {
        VoiceHandle voice = handleOf(*source);
        if (voice.valid()) {
            m_audio->stop(voice);
            clearHandle(*source);
        }
    }
}

Eigen::Vector3f AudioSystem::velocityFrom(const Eigen::Vector3f& current, Eigen::Vector3f& last, bool& has_last, double delta) {
    if (!has_last || delta <= 0.0) {
        // First sighting (or a stalled frame): reporting (current - 0) / dt would be an enormous
        // bogus velocity and an audible Doppler shriek on spawn.
        last = current;
        has_last = true;
        return Eigen::Vector3f::Zero();
    }
    const Eigen::Vector3f velocity = (current - last) / static_cast<float>(delta);
    last = current;
    return velocity;
}

}  // namespace ICE
