#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Asset.h"

namespace ICE {

// A CPU-side sound: interleaved signed 16-bit PCM plus its format. This is the audio counterpart
// of Mesh/Texture -- pure data, no device object. The backend-resident buffer uploaded from it is
// owned by AudioRegistry, exactly as GPURegistry owns the GPU upload of a Mesh.
//
// 16-bit interleaved is the one format every OpenAL implementation accepts without extensions, so
// the decoders normalize to it (see AudioDecoder.h) and nothing downstream has to convert.
class AudioClip : public Asset {
   public:
    AudioClip() = default;
    AudioClip(std::vector<int16_t> samples, uint32_t channels, uint32_t sampleRate);

    // Streaming clip: format and length are known from the file header, but no PCM is resident --
    // it is decoded incrementally at playback time. This is how multi-minute music avoids holding
    // tens of megabytes of samples in RAM. `frameCount` may be 0 if the format cannot report a
    // length cheaply; nothing depends on it being exact.
    static AudioClip Streaming(uint32_t channels, uint32_t sampleRate, uint64_t frameCount);

    AssetType getType() const override { return AssetType::EAudioClip; }
    std::string getTypeName() const override { return "AudioClip"; }

    const std::vector<int16_t>& samples() const { return m_samples; }
    uint32_t getChannels() const { return m_channels; }
    uint32_t getSampleRate() const { return m_sample_rate; }

    uint64_t getFrameCount() const;
    double getDuration() const;
    std::size_t sizeBytes() const { return m_samples.size() * sizeof(int16_t); }

    // OpenAL only spatializes MONO buffers -- a stereo buffer plays flat, at full volume,
    // ignoring listener and source position entirely. This is the single most common "why is my
    // 3D audio not working" cause, so the check is part of the asset's public surface and is
    // enforced where a clip is used spatially rather than being left to callers to remember.
    bool isMono() const { return m_channels == 1; }

    // Decoded on demand rather than held in memory. A streaming clip has no samples() to upload:
    // playback goes through IAudioStream and a queued-buffer voice instead.
    bool isStreaming() const { return m_streaming; }

    // A clip with nothing usable: a failed decode, or a default-constructed placeholder. A
    // streaming clip is NOT empty despite having no samples -- its content lives in its source
    // file, so emptiness is decided by the format instead.
    bool isEmpty() const { return m_streaming ? m_channels == 0 : (m_samples.empty() || m_channels == 0); }

   private:
    std::vector<int16_t> m_samples;  // interleaved, channels * frameCount entries; empty if streaming
    uint32_t m_channels = 0;
    uint32_t m_sample_rate = 0;
    bool m_streaming = false;
    uint64_t m_streaming_frames = 0;  // header-reported length; only meaningful when streaming
};

}  // namespace ICE
