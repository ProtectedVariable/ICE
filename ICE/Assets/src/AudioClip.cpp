#include "AudioClip.h"

namespace ICE {

AudioClip::AudioClip(std::vector<int16_t> samples, uint32_t channels, uint32_t sampleRate)
    : m_samples(std::move(samples)),
      m_channels(channels),
      m_sample_rate(sampleRate) {}

uint64_t AudioClip::getFrameCount() const {
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
