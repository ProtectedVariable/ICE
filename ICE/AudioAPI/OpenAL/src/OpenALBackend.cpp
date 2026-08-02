#include "OpenALBackend.h"

#include <AudioClip.h>
#include <HandlePool.h>
#include <IAudioStream.h>
#include <JobScheduler.h>

#include <algorithm>
#include <array>
#include <atomic>
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

// Streaming tuning. Four chunks of ~0.35s gives well over a second of buffered audio -- enough to
// absorb a stalled frame or a slow decode without the source running dry, while keeping the memory
// per streaming voice modest (a few hundred KB).
constexpr std::size_t kStreamChunks = 4;
constexpr double kStreamChunkSeconds = 0.35;

// Per-chunk handoff between the decode job (producer) and update() (consumer).
enum class ChunkState : uint8_t { Empty, Filling, Ready };

// Shared state for one streaming voice, held by shared_ptr so an in-flight decode job keeps it
// alive even if the voice is released mid-decode. That is what makes teardown lock-free: release
// just sets `cancelled` and drops the backend's reference; the job observes the flag, stops, and
// the state dies with the last reference. Nothing ever joins a decode job, so nothing can deadlock.
struct StreamState {
    std::shared_ptr<IAudioStream> stream;

    struct Chunk {
        std::vector<int16_t> pcm;                          // sized for kStreamChunkSeconds
        uint64_t frames = 0;                               // valid frames in pcm
        std::atomic<ChunkState> state{ChunkState::Empty};  // producer/consumer handoff
    };
    std::array<Chunk, kStreamChunks> chunks;

    std::atomic<bool> decoding{false};   // exactly one decode job in flight at a time
    std::atomic<bool> cancelled{false};  // set by releaseVoice; the job bails out
    std::atomic<bool> eof{false};        // stream exhausted and not looping
    bool looping = false;

    uint32_t channels = 0;
    uint32_t sample_rate = 0;
    ALenum format = AL_FORMAT_MONO16;
};

// A voice is a borrowed source id plus the buffer it is playing. The id comes from the fixed set
// allocated at initialize() and returns to the free list on release. A streaming voice instead
// owns its own queue of AL buffers, fed from `stream`.
struct OpenALVoice {
    ALuint source = 0;
    AudioBufferHandle buffer;

    // Streaming only (null for a resident voice).
    std::shared_ptr<StreamState> stream;
    std::vector<ALuint> queue_buffers;  // owned by this voice, deleted on release
    std::vector<ALuint> free_buffers;   // subset of queue_buffers not currently queued on the source
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

    std::shared_ptr<JobScheduler> scheduler;  // optional; decode runs inline without it
    std::size_t underruns = 0;
};

namespace {
// Decode into every Empty chunk we can, in place. Runs on a worker thread (or inline when there is
// no scheduler) and touches ONLY the stream and its staging chunks -- never OpenAL. That is the
// invariant that keeps every al* call on the main thread.
void decodeChunks(const std::shared_ptr<StreamState>& state) {
    for (auto& chunk : state->chunks) {
        if (state->cancelled.load(std::memory_order_acquire)) {
            break;
        }
        ChunkState expected = ChunkState::Empty;
        if (!chunk.state.compare_exchange_strong(expected, ChunkState::Filling, std::memory_order_acq_rel)) {
            continue;  // already Ready, or being consumed
        }

        const uint64_t capacity_frames = chunk.pcm.size() / state->channels;
        uint64_t got = state->stream->read(chunk.pcm.data(), capacity_frames);

        // End of the sound: rewind and keep filling the same chunk so the loop point falls inside
        // a chunk rather than leaving a silent gap at the queue boundary.
        if (got < capacity_frames) {
            if (state->looping) {
                state->stream->rewind();
                while (got < capacity_frames) {
                    const uint64_t more = state->stream->read(chunk.pcm.data() + got * state->channels, capacity_frames - got);
                    if (more == 0) {
                        break;  // empty or unreadable stream; avoid spinning forever
                    }
                    got += more;
                }
            } else if (got == 0) {
                state->eof.store(true, std::memory_order_release);
                chunk.state.store(ChunkState::Empty, std::memory_order_release);
                break;
            }
        }

        chunk.frames = got;
        chunk.state.store(got > 0 ? ChunkState::Ready : ChunkState::Empty, std::memory_order_release);
    }
    state->decoding.store(false, std::memory_order_release);
}
}  // namespace

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

VoiceHandle OpenALBackend::acquireStreamingVoice(const std::shared_ptr<IAudioStream>& stream, const VoiceDesc& desc) {
    if (!isAvailable() || stream == nullptr || !stream->isValid() || m_impl->free_sources.empty()) {
        return {};
    }
    const uint32_t channels = stream->getChannels();
    const uint32_t rate = stream->getSampleRate();
    if (channels == 0 || rate == 0) {
        return {};
    }

    auto state = std::make_shared<StreamState>();
    state->stream = stream;
    state->channels = channels;
    state->sample_rate = rate;
    state->format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    state->looping = desc.params.looping;

    const auto frames_per_chunk = static_cast<std::size_t>(kStreamChunkSeconds * rate);
    for (auto& chunk : state->chunks) {
        chunk.pcm.resize(frames_per_chunk * channels);
    }

    // Prime synchronously so playback can begin this frame rather than after a decode round-trip.
    decodeChunks(state);

    const ALuint source = m_impl->free_sources.back();
    m_impl->free_sources.pop_back();

    OpenALVoice voice;
    voice.source = source;
    voice.stream = state;
    voice.queue_buffers.resize(kStreamChunks);
    alGetError();
    alGenBuffers(static_cast<ALsizei>(kStreamChunks), voice.queue_buffers.data());
    if (ALenum err = alGetError(); err != AL_NO_ERROR) {
        Logger::Log(Logger::ERROR, "Audio", "alGenBuffers for a streaming voice failed: %s", alErrorString(err));
        m_impl->free_sources.push_back(source);
        return {};
    }
    // All of this voice's buffers start unqueued and available to fill.
    voice.free_buffers = voice.queue_buffers;

    // A streaming source must have no static buffer attached, and AL_LOOPING must stay off: on a
    // queued-buffer source it would loop the individual queued chunk instead of the sound.
    AL_CHECK(alSourcei(source, AL_BUFFER, 0));
    AL_CHECK(alSourcei(source, AL_LOOPING, AL_FALSE));

    VoiceHandle handle = m_impl->voices.insert(std::move(voice));
    setVoiceParams(handle, desc.params);
    pumpStream(handle);  // queue the primed chunks
    return handle;
}

// Move decoded chunks into the source's AL queue and keep the decoder ahead of playback. Main
// thread only -- every al* call in the streaming path lives here.
void OpenALBackend::pumpStream(VoiceHandle handle) {
    OpenALVoice* v = m_impl->voices.get(handle);
    if (v == nullptr || v->stream == nullptr) {
        return;
    }
    auto& state = v->stream;

    // 1. Reclaim buffers the device has finished with.
    ALint processed = 0;
    alGetSourcei(v->source, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0) {
        ALuint done = 0;
        AL_CHECK(alSourceUnqueueBuffers(v->source, 1, &done));
        v->free_buffers.push_back(done);
    }

    // 2. Queue every ready chunk into a free buffer.
    for (auto& chunk : state->chunks) {
        if (v->free_buffers.empty()) {
            break;
        }
        if (chunk.state.load(std::memory_order_acquire) != ChunkState::Ready) {
            continue;
        }
        const ALuint buffer = v->free_buffers.back();
        v->free_buffers.pop_back();

        AL_CHECK(alBufferData(buffer, state->format, chunk.pcm.data(),
                              static_cast<ALsizei>(chunk.frames * state->channels * sizeof(int16_t)),
                              static_cast<ALsizei>(state->sample_rate)));
        AL_CHECK(alSourceQueueBuffers(v->source, 1, &buffer));
        chunk.state.store(ChunkState::Empty, std::memory_order_release);
    }

    // 3. Keep the decoder ahead. One job at a time per stream, so the stream object is never
    //    touched concurrently and needs no lock of its own.
    bool expected = false;
    if (!state->eof.load(std::memory_order_acquire) && state->decoding.compare_exchange_strong(expected, true)) {
        auto captured = state;  // shared_ptr: outlives the voice if it is released mid-decode
        if (m_impl->scheduler) {
            m_impl->scheduler->submit([captured] { decodeChunks(captured); });
        } else {
            decodeChunks(captured);  // no scheduler: correct, but spikes this frame
        }
    }

    // 4. Underrun recovery. A source that ran dry stops on its own; restart it once audio is
    //    queued again. Without this a single late refill would silence the music permanently.
    ALint queued = 0;
    ALint state_al = 0;
    alGetSourcei(v->source, AL_BUFFERS_QUEUED, &queued);
    alGetSourcei(v->source, AL_SOURCE_STATE, &state_al);
    if (queued > 0 && state_al == AL_STOPPED) {
        ++m_impl->underruns;
        AL_CHECK(alSourcePlay(v->source));
    }
}

void OpenALBackend::releaseVoice(VoiceHandle voice) {
    OpenALVoice* v = m_impl->voices.get(voice);
    if (v == nullptr) {
        return;  // stale handle -- exactly what the generation check is for
    }
    AL_CHECK(alSourceStop(v->source));

    if (v->stream != nullptr) {
        // Tell any in-flight decode job to stop. We do NOT wait for it: the job holds its own
        // shared_ptr to the stream state, so it can finish harmlessly against state that no longer
        // belongs to a voice. Nothing joins, so a stop during a refill cannot deadlock.
        v->stream->cancelled.store(true, std::memory_order_release);

        // Unqueue everything before deleting: OpenAL refuses to delete a queued buffer.
        ALint processed = 0;
        alGetSourcei(v->source, AL_BUFFERS_PROCESSED, &processed);
        while (processed-- > 0) {
            ALuint done = 0;
            alSourceUnqueueBuffers(v->source, 1, &done);
        }
        AL_CHECK(alSourcei(v->source, AL_BUFFER, 0));  // detaches any still-queued buffers
        if (!v->queue_buffers.empty()) {
            AL_CHECK(alDeleteBuffers(static_cast<ALsizei>(v->queue_buffers.size()), v->queue_buffers.data()));
        }
    } else {
        AL_CHECK(alSourcei(v->source, AL_BUFFER, 0));  // detach so the buffer can be deleted later
    }

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
    if (v->stream != nullptr) {
        // Never set AL_LOOPING on a queued-buffer source: it would loop whichever chunk is
        // currently queued instead of the sound. Streamed looping is the decoder rewinding.
        v->stream->looping = params.looping;
    } else {
        AL_CHECK(alSourcei(source, AL_LOOPING, params.looping ? AL_TRUE : AL_FALSE));
    }

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
    if (state == AL_PLAYING || state == AL_PAUSED) {
        return true;
    }

    // A streaming voice that momentarily ran dry reports AL_STOPPED even though the sound is not
    // over. Reporting it inactive would make AudioEngine reclaim it and the music would vanish on
    // the first hitch. It is finished only once the decoder hit EOF *and* the queue has drained.
    if (v->stream != nullptr && !v->stream->eof.load(std::memory_order_acquire)) {
        return true;
    }
    if (v->stream != nullptr) {
        ALint queued = 0;
        alGetSourcei(v->source, AL_BUFFERS_QUEUED, &queued);
        ALint processed = 0;
        alGetSourcei(v->source, AL_BUFFERS_PROCESSED, &processed);
        return queued > processed;  // audio still pending on the device
    }
    return false;
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
    // Mixing runs on OpenAL's own thread and finished-voice reclamation is driven by AudioEngine
    // polling isVoiceActive. What remains here is the streaming refill: all of it main-thread, with
    // only the decode itself pushed onto the scheduler.
    if (!isAvailable()) {
        return;
    }
    std::vector<VoiceHandle> streaming;
    m_impl->voices.forEachHandle([&](VoiceHandle handle, OpenALVoice& voice) {
        if (voice.stream != nullptr) {
            streaming.push_back(handle);
        }
    });
    for (VoiceHandle handle : streaming) {
        pumpStream(handle);
    }
}

void OpenALBackend::setScheduler(const std::shared_ptr<JobScheduler>& scheduler) {
    m_impl->scheduler = scheduler;
}

std::size_t OpenALBackend::streamUnderrunCount() const {
    return m_impl->underruns;
}

}  // namespace ICE
