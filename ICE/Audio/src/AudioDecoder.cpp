#include "AudioDecoder.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

// Declarations only -- every implementation macro is defined exactly once, in
// audio_decoders_impl.cpp. Including these without the macros yields the prototypes.
#include <dr_flac.h>
#include <dr_mp3.h>
#include <dr_wav.h>

// stb_vorbis has no separate header; STB_VORBIS_HEADER_ONLY is how it exposes just the prototypes.
// Without it this TU would emit a second copy of every stb_vorbis symbol and fail to link.
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#undef STB_VORBIS_HEADER_ONLY

namespace ICE {
namespace {

// Adopt a decoder-owned buffer of interleaved int16 into a DecodedAudio, then release it with the
// decoder's own deallocator. Every branch below produces its buffer the same way, so the copy and
// the free live in one place.
template<typename FreeFn>
DecodedAudio adopt(const int16_t* data, uint32_t channels, uint32_t sampleRate, uint64_t frames, FreeFn&& release) {
    DecodedAudio out;
    out.channels = channels;
    out.sampleRate = sampleRate;
    out.frameCount = frames;
    out.samples.assign(data, data + frames * channels);
    release();
    return out;
}

std::string lowerExtension(const std::filesystem::path& file) {
    std::string ext = file.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

}  // namespace

const std::vector<std::string>& SupportedAudioExtensions() {
    static const std::vector<std::string> extensions = {".wav", ".mp3", ".flac", ".ogg"};
    return extensions;
}

void DownmixToMono(DecodedAudio& audio) {
    if (audio.channels <= 1 || audio.samples.empty()) {
        return;
    }
    const uint32_t channels = audio.channels;
    const std::size_t frames = audio.samples.size() / channels;

    std::vector<int16_t> mono(frames);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        // Accumulate in int32: summing several int16 channels overflows 16 bits before the divide.
        int32_t sum = 0;
        for (uint32_t c = 0; c < channels; ++c) {
            sum += audio.samples[frame * channels + c];
        }
        mono[frame] = static_cast<int16_t>(sum / static_cast<int32_t>(channels));
    }

    audio.samples = std::move(mono);
    audio.channels = 1;
    audio.frameCount = frames;
}

std::optional<DecodedAudio> DecodeAudioFile(const std::filesystem::path& file) {
    const std::string ext = lowerExtension(file);
    const std::string path = file.string();

    if (ext == ".wav") {
        unsigned int channels = 0;
        unsigned int sampleRate = 0;
        drwav_uint64 frames = 0;
        drwav_int16* data = drwav_open_file_and_read_pcm_frames_s16(path.c_str(), &channels, &sampleRate, &frames, nullptr);
        if (data == nullptr) {
            return std::nullopt;
        }
        return adopt(data, channels, sampleRate, frames, [data] { drwav_free(data, nullptr); });
    }

    if (ext == ".mp3") {
        drmp3_config config{};
        drmp3_uint64 frames = 0;
        drmp3_int16* data = drmp3_open_file_and_read_pcm_frames_s16(path.c_str(), &config, &frames, nullptr);
        if (data == nullptr) {
            return std::nullopt;
        }
        return adopt(data, config.channels, config.sampleRate, frames, [data] { drmp3_free(data, nullptr); });
    }

    if (ext == ".flac") {
        unsigned int channels = 0;
        unsigned int sampleRate = 0;
        drflac_uint64 frames = 0;
        drflac_int16* data = drflac_open_file_and_read_pcm_frames_s16(path.c_str(), &channels, &sampleRate, &frames, nullptr);
        if (data == nullptr) {
            return std::nullopt;
        }
        return adopt(data, channels, sampleRate, frames, [data] { drflac_free(data, nullptr); });
    }

    if (ext == ".ogg") {
        int channels = 0;
        int sampleRate = 0;
        short* data = nullptr;
        // Returns the frame count, or -1 on failure. Allocates with malloc, so it frees with free.
        const int frames = stb_vorbis_decode_filename(path.c_str(), &channels, &sampleRate, &data);
        if (frames < 0 || data == nullptr) {
            return std::nullopt;
        }
        return adopt(data, static_cast<uint32_t>(channels), static_cast<uint32_t>(sampleRate), static_cast<uint64_t>(frames),
                     [data] { std::free(data); });
    }

    return std::nullopt;
}

}  // namespace ICE
