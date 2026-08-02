#include <Logger.h>

#include <algorithm>
#include <cctype>
#include <string>

#include "IAudioStream.h"

// Declarations only -- the implementations live in audio_decoders_impl.cpp.
#include <dr_flac.h>
#include <dr_mp3.h>
#include <dr_wav.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#undef STB_VORBIS_HEADER_ONLY

namespace ICE {
namespace {

std::string lowerExtension(const std::filesystem::path& file) {
    std::string ext = file.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

// Each decoder keeps its own open handle; the shape is identical, so the differences are confined
// to these four small classes rather than leaking into the streaming machinery.

class WavStream : public IAudioStream {
   public:
    explicit WavStream(const std::filesystem::path& file) { m_ok = drwav_init_file(&m_wav, file.string().c_str(), nullptr) == DRWAV_TRUE; }
    ~WavStream() override {
        if (m_ok) {
            drwav_uninit(&m_wav);
        }
    }
    uint32_t getChannels() const override { return m_ok ? m_wav.channels : 0; }
    uint32_t getSampleRate() const override { return m_ok ? m_wav.sampleRate : 0; }
    uint64_t getTotalFrames() const override { return m_ok ? m_wav.totalPCMFrameCount : 0; }
    uint64_t read(int16_t* out, uint64_t maxFrames) override { return m_ok ? drwav_read_pcm_frames_s16(&m_wav, maxFrames, out) : 0; }
    void rewind() override {
        if (m_ok) {
            drwav_seek_to_pcm_frame(&m_wav, 0);
        }
    }
    bool isValid() const override { return m_ok; }

   private:
    drwav m_wav{};
    bool m_ok = false;
};

class Mp3Stream : public IAudioStream {
   public:
    explicit Mp3Stream(const std::filesystem::path& file) { m_ok = drmp3_init_file(&m_mp3, file.string().c_str(), nullptr) == DRMP3_TRUE; }
    ~Mp3Stream() override {
        if (m_ok) {
            drmp3_uninit(&m_mp3);
        }
    }
    uint32_t getChannels() const override { return m_ok ? m_mp3.channels : 0; }
    uint32_t getSampleRate() const override { return m_ok ? m_mp3.sampleRate : 0; }
    // Counting MP3 frames requires a full scan, so it is done once here rather than per call.
    uint64_t getTotalFrames() const override {
        if (!m_ok) {
            return 0;
        }
        if (m_total_frames == 0) {
            m_total_frames = drmp3_get_pcm_frame_count(&m_mp3);
            drmp3_seek_to_pcm_frame(&m_mp3, 0);  // the scan moved the read cursor
        }
        return m_total_frames;
    }
    uint64_t read(int16_t* out, uint64_t maxFrames) override { return m_ok ? drmp3_read_pcm_frames_s16(&m_mp3, maxFrames, out) : 0; }
    void rewind() override {
        if (m_ok) {
            drmp3_seek_to_pcm_frame(&m_mp3, 0);
        }
    }
    bool isValid() const override { return m_ok; }

   private:
    mutable drmp3 m_mp3{};
    mutable uint64_t m_total_frames = 0;
    bool m_ok = false;
};

class FlacStream : public IAudioStream {
   public:
    explicit FlacStream(const std::filesystem::path& file) { m_flac = drflac_open_file(file.string().c_str(), nullptr); }
    ~FlacStream() override {
        if (m_flac != nullptr) {
            drflac_close(m_flac);
        }
    }
    uint32_t getChannels() const override { return m_flac ? m_flac->channels : 0; }
    uint32_t getSampleRate() const override { return m_flac ? m_flac->sampleRate : 0; }
    uint64_t getTotalFrames() const override { return m_flac ? m_flac->totalPCMFrameCount : 0; }
    uint64_t read(int16_t* out, uint64_t maxFrames) override { return m_flac ? drflac_read_pcm_frames_s16(m_flac, maxFrames, out) : 0; }
    void rewind() override {
        if (m_flac != nullptr) {
            drflac_seek_to_pcm_frame(m_flac, 0);
        }
    }
    bool isValid() const override { return m_flac != nullptr; }

   private:
    drflac* m_flac = nullptr;
};

class VorbisStream : public IAudioStream {
   public:
    explicit VorbisStream(const std::filesystem::path& file) {
        int error = 0;
        m_vorbis = stb_vorbis_open_filename(file.string().c_str(), &error, nullptr);
        if (m_vorbis != nullptr) {
            m_info = stb_vorbis_get_info(m_vorbis);
        }
    }
    ~VorbisStream() override {
        if (m_vorbis != nullptr) {
            stb_vorbis_close(m_vorbis);
        }
    }
    uint32_t getChannels() const override { return m_vorbis ? static_cast<uint32_t>(m_info.channels) : 0; }
    uint32_t getSampleRate() const override { return m_vorbis ? m_info.sample_rate : 0; }
    uint64_t getTotalFrames() const override { return m_vorbis ? stb_vorbis_stream_length_in_samples(m_vorbis) : 0; }
    uint64_t read(int16_t* out, uint64_t maxFrames) override {
        if (m_vorbis == nullptr) {
            return 0;
        }
        // Returns frames (not shorts) despite taking a short count.
        const int frames = stb_vorbis_get_samples_short_interleaved(m_vorbis, m_info.channels, out,
                                                                    static_cast<int>(maxFrames * m_info.channels));
        return frames < 0 ? 0 : static_cast<uint64_t>(frames);
    }
    void rewind() override {
        if (m_vorbis != nullptr) {
            stb_vorbis_seek_start(m_vorbis);
        }
    }
    bool isValid() const override { return m_vorbis != nullptr; }

   private:
    stb_vorbis* m_vorbis = nullptr;
    stb_vorbis_info m_info{};
};

}  // namespace

std::shared_ptr<IAudioStream> OpenAudioFileStream(const std::filesystem::path& file) {
    const std::string ext = lowerExtension(file);

    std::shared_ptr<IAudioStream> stream;
    if (ext == ".wav") {
        stream = std::make_shared<WavStream>(file);
    } else if (ext == ".mp3") {
        stream = std::make_shared<Mp3Stream>(file);
    } else if (ext == ".flac") {
        stream = std::make_shared<FlacStream>(file);
    } else if (ext == ".ogg") {
        stream = std::make_shared<VorbisStream>(file);
    } else {
        Logger::Log(Logger::ERROR, "Audio", "Cannot stream '%s': unsupported format.", file.string().c_str());
        return nullptr;
    }

    if (!stream->isValid()) {
        Logger::Log(Logger::ERROR, "Audio", "Could not open '%s' for streaming.", file.string().c_str());
        return nullptr;
    }
    return stream;
}

}  // namespace ICE
