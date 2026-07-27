#include "OpenALBackend.h"

#include <AudioClip.h>
#include <HandlePool.h>

#include <algorithm>
#include <vector>

#include "ALCheck.h"

namespace ICE {
namespace {

ALenum distanceModelFor(AttenuationModel model) {
    switch (model) {
        case AttenuationModel::None: return AL_NONE;
        case AttenuationModel::LinearDistance: return AL_LINEAR_DISTANCE_CLAMPED;
        case AttenuationModel::ExponentDistance: return AL_EXPONENT_DISTANCE_CLAMPED;
        case AttenuationModel::InverseDistance:
        default: return AL_INVERSE_DISTANCE_CLAMPED;
    }
}

ALenum formatFor(const AudioClip& clip) {
    // The decoders normalize everything to 16-bit, so only the channel count varies here.
    return clip.getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
}

}  // namespace

// A voice is a borrowed source id plus the buffer it is playing. The id comes from the fixed set
// allocated at initialize() and returns to the free list on release.
struct OpenALVoice {
    ALuint source = 0;
    AudioBufferHandle buffer;
};

struct OpenALBackend::Impl {
    OpenALDevice device;
    AudioDeviceConfig config;
    bool initialized = false;

    HandlePool<ALuint, AudioBufferTag> buffers;
    HandlePool<OpenALVoice, VoiceTag> voices;

    // Source ids allocated up front and handed out by acquireVoice.
    std::vector<ALuint> all_sources;
    std::vector<ALuint> free_sources;
};

OpenALBackend::OpenALBackend() : m_impl(std::make_unique<Impl>()) {}

OpenALBackend::~OpenALBackend() {
    shutdown();
}

bool OpenALBackend::initialize(const AudioDeviceConfig& config) {
    if (m_impl->initialized) {
        return true;
    }
    m_impl->config = config;

    if (!m_impl->device.open(config)) {
        return false;  // caller falls back to NullAudioBackend
    }

    // Allocate the whole source set now. The device reports its own ceiling, so never ask for more
    // than it will give -- alGenSources would fail and leave us in a half-allocated state.
    const auto& info = m_impl->device.info();
    const int requested = config.monoVoices + config.stereoVoices;
    const int available = info.monoSources > 0 ? info.monoSources : requested;
    const int count = std::min(requested, available);

    m_impl->all_sources.resize(static_cast<std::size_t>(count));
    alGetError();  // clear any stale flag before a call whose result we branch on
    alGenSources(count, m_impl->all_sources.data());
    if (ALenum err = alGetError(); err != AL_NO_ERROR) {
        Logger::Log(Logger::ERROR, "Audio", "alGenSources(%d) failed: %s -- audio disabled.", count, alErrorString(err));
        m_impl->all_sources.clear();
        m_impl->device.close();
        return false;
    }
    m_impl->free_sources.assign(m_impl->all_sources.begin(), m_impl->all_sources.end());

    // Distance model is a GLOBAL context setting in OpenAL, not per-source -- per-source you only
    // get reference/max distance and rolloff. Hence a project-wide setting rather than a
    // per-component one.
    AL_CHECK(alDistanceModel(distanceModelFor(config.attenuation)));
    AL_CHECK(alDopplerFactor(config.dopplerFactor));
    AL_CHECK(alSpeedOfSound(config.speedOfSound));

    m_impl->initialized = true;
    Logger::Log(Logger::INFO, "Audio", "OpenAL backend ready: %d voices on '%s'.", count, info.deviceName.c_str());
    return true;
}

void OpenALBackend::shutdown() {
    if (!m_impl->initialized) {
        return;
    }
    // Sources first (they reference buffers), then buffers, then the device.
    for (ALuint source : m_impl->all_sources) {
        alSourceStop(source);
        alSourcei(source, AL_BUFFER, 0);
    }
    if (!m_impl->all_sources.empty()) {
        alDeleteSources(static_cast<ALsizei>(m_impl->all_sources.size()), m_impl->all_sources.data());
    }
    m_impl->all_sources.clear();
    m_impl->free_sources.clear();
    m_impl->voices = {};

    // AudioRegistry::clear() releases buffers before the backend is torn down, so normally there
    // is nothing left here. Delete any stragglers rather than leaking them into the driver.
    std::vector<ALuint> leftover;
    m_impl->buffers.forEachHandle([&](AudioBufferHandle, ALuint& id) { leftover.push_back(id); });
    if (!leftover.empty()) {
        Logger::Log(Logger::DEBUG, "Audio", "Releasing %zu audio buffer(s) still resident at shutdown.", leftover.size());
        alDeleteBuffers(static_cast<ALsizei>(leftover.size()), leftover.data());
    }
    m_impl->buffers = {};

    m_impl->device.close();
    m_impl->initialized = false;
}

bool OpenALBackend::isAvailable() const {
    return m_impl->initialized && m_impl->device.isOpen();
}

std::string OpenALBackend::deviceName() const {
    return m_impl->device.info().deviceName;
}

const OpenALDeviceInfo& OpenALBackend::deviceInfo() const {
    return m_impl->device.info();
}

AudioBufferHandle OpenALBackend::uploadClip(const AudioClip& clip) {
    if (!isAvailable() || clip.isEmpty()) {
        return {};
    }

    ALuint id = 0;
    alGetError();
    alGenBuffers(1, &id);
    if (ALenum err = alGetError(); err != AL_NO_ERROR) {
        Logger::Log(Logger::ERROR, "Audio", "alGenBuffers failed: %s", alErrorString(err));
        return {};
    }

    const auto& samples = clip.samples();
    alBufferData(id, formatFor(clip), samples.data(), static_cast<ALsizei>(samples.size() * sizeof(int16_t)),
                 static_cast<ALsizei>(clip.getSampleRate()));
    if (ALenum err = alGetError(); err != AL_NO_ERROR) {
        Logger::Log(Logger::ERROR, "Audio", "alBufferData failed: %s", alErrorString(err));
        alDeleteBuffers(1, &id);
        return {};
    }

    return m_impl->buffers.insert(id);
}

void OpenALBackend::releaseBuffer(AudioBufferHandle buffer) {
    ALuint* id = m_impl->buffers.get(buffer);
    if (id == nullptr) {
        return;
    }

    // OpenAL refuses to delete a buffer that is still attached to a source, so stop and detach any
    // voice playing it first. This is the path an asset re-import takes (AssetBank fires its
    // removal listener -> AudioRegistry::evict -> here) while the old sound may still be audible.
    std::vector<VoiceHandle> playing;
    m_impl->voices.forEachHandle([&](VoiceHandle handle, OpenALVoice& voice) {
        if (voice.buffer == buffer) {
            playing.push_back(handle);
        }
    });
    for (VoiceHandle handle : playing) {
        releaseVoice(handle);
    }

    AL_CHECK(alDeleteBuffers(1, id));
    m_impl->buffers.erase(buffer);
}

VoiceHandle OpenALBackend::acquireVoice(AudioBufferHandle buffer, const VoiceDesc& desc) {
    if (!isAvailable() || m_impl->free_sources.empty()) {
        return {};  // out of voices -- AudioEngine decides whether to steal
    }
    ALuint* buffer_id = m_impl->buffers.get(buffer);
    if (buffer_id == nullptr) {
        return {};
    }

    const ALuint source = m_impl->free_sources.back();
    m_impl->free_sources.pop_back();

    AL_CHECK(alSourcei(source, AL_BUFFER, static_cast<ALint>(*buffer_id)));
    VoiceHandle handle = m_impl->voices.insert(OpenALVoice{source, buffer});
    setVoiceParams(handle, desc.params);
    return handle;
}

void OpenALBackend::releaseVoice(VoiceHandle voice) {
    OpenALVoice* v = m_impl->voices.get(voice);
    if (v == nullptr) {
        return;  // stale handle -- exactly what the generation check is for
    }
    AL_CHECK(alSourceStop(v->source));
    AL_CHECK(alSourcei(v->source, AL_BUFFER, 0));  // detach so the buffer can be deleted later
    m_impl->free_sources.push_back(v->source);
    m_impl->voices.erase(voice);
}

void OpenALBackend::setVoiceParams(VoiceHandle voice, const VoiceParams& params) {
    OpenALVoice* v = m_impl->voices.get(voice);
    if (v == nullptr) {
        return;
    }
    const ALuint source = v->source;

    AL_CHECK(alSourcef(source, AL_GAIN, params.gain));
    AL_CHECK(alSourcef(source, AL_PITCH, params.pitch));
    AL_CHECK(alSourcei(source, AL_LOOPING, params.looping ? AL_TRUE : AL_FALSE));

    if (params.spatial) {
        AL_CHECK(alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE));
        AL_CHECK(alSource3f(source, AL_POSITION, params.position.x(), params.position.y(), params.position.z()));
        AL_CHECK(alSource3f(source, AL_VELOCITY, params.velocity.x(), params.velocity.y(), params.velocity.z()));
        AL_CHECK(alSourcef(source, AL_REFERENCE_DISTANCE, params.minDistance));
        AL_CHECK(alSourcef(source, AL_MAX_DISTANCE, params.maxDistance));
        AL_CHECK(alSourcef(source, AL_ROLLOFF_FACTOR, params.rolloff));
    } else {
        // Head-relative at the origin: plays flat at full gain regardless of where the listener is
        // or faces. This is the correct mode for music and UI, and also where a stereo clip ends
        // up (OpenAL will not spatialize one).
        AL_CHECK(alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE));
        AL_CHECK(alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f));
        AL_CHECK(alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f));
        AL_CHECK(alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f));
    }
}

void OpenALBackend::setVoiceState(VoiceHandle voice, PlaybackState state) {
    OpenALVoice* v = m_impl->voices.get(voice);
    if (v == nullptr) {
        return;
    }
    switch (state) {
        case PlaybackState::Playing: AL_CHECK(alSourcePlay(v->source)); break;
        case PlaybackState::Paused: AL_CHECK(alSourcePause(v->source)); break;
        case PlaybackState::Stopped: AL_CHECK(alSourceStop(v->source)); break;
    }
}

bool OpenALBackend::isVoiceActive(VoiceHandle voice) const {
    const OpenALVoice* v = m_impl->voices.get(voice);
    if (v == nullptr) {
        return false;
    }
    ALint state = 0;
    alGetSourcei(v->source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING || state == AL_PAUSED;
}

std::size_t OpenALBackend::activeVoiceCount() const {
    return m_impl->voices.size();
}

std::size_t OpenALBackend::voiceCapacity() const {
    return m_impl->all_sources.size();
}

void OpenALBackend::setListener(const ListenerState& listener) {
    if (!isAvailable()) {
        return;
    }
    AL_CHECK(alListener3f(AL_POSITION, listener.position.x(), listener.position.y(), listener.position.z()));
    AL_CHECK(alListener3f(AL_VELOCITY, listener.velocity.x(), listener.velocity.y(), listener.velocity.z()));
    AL_CHECK(alListenerf(AL_GAIN, listener.gain));

    // AL_ORIENTATION is six floats: the "at" vector followed by "up".
    const ALfloat orientation[6] = {listener.forward.x(), listener.forward.y(), listener.forward.z(),
                                    listener.up.x(),      listener.up.y(),      listener.up.z()};
    AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
}

void OpenALBackend::update(double /*delta*/) {
    // Nothing to do in phase 1: mixing runs on OpenAL's own thread and finished-voice reclamation
    // is driven by AudioEngine polling isVoiceActive. Phase 4's streaming refill hooks in here.
}

}  // namespace ICE
