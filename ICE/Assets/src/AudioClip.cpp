#include "AudioClip.h"

namespace ICE {

AudioClip::AudioClip(std::vector<int16_t> samples, uint32_t channels, uint32_t sampleRate)
    : m_samples(std::move(samples)),
      m_channels(channels),
      m_sample_rate(sampleRate) {}

AudioClip AudioClip::Streaming(uint32_t channels, uint32_t sampleRate, uint64_t frameCount) {
    AudioClip clip;
    clip.m_channels = channels;
    clip.m_sample_rate = sampleRate;
    clip.m_streaming = true;
    clip.m_streaming_frames = frameCount;
    return clip;
}

uint64_t AudioClip::getFrameCount() const {
    if (m_streaming) {
        return m_streaming_frames;  // from the file header; no samples are resident
    }
    if (m_channels == 0) {
        return 0;
    }
    return static_cast<uint64_t>(m_samples.size()) / m_channels;
}

double AudioClip::getDuration() const {
    if (m_sample_rate == 0) {
        return 0.0;
    }
    return static_cast<double>(getFrameCount()) / m_sample_rate;
}

}  // namespace ICE
