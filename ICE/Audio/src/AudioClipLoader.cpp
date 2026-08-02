#include "AudioClipLoader.h"

#include <Logger.h>

#include "AudioDecoder.h"
#include "IAudioStream.h"

namespace ICE {
namespace {
// 1 MiB of encoded audio is roughly 30-60s of MP3/Vorbis -- comfortably past the point where
// holding the decoded PCM resident stops being worthwhile.
std::size_t g_streaming_threshold_bytes = 1024 * 1024;
}  // namespace

std::size_t AudioClipLoader::streamingThresholdBytes() {
    return g_streaming_threshold_bytes;
}

void AudioClipLoader::setStreamingThresholdBytes(std::size_t bytes) {
    g_streaming_threshold_bytes = bytes;
}

std::shared_ptr<AudioClip> AudioClipLoader::load(const std::vector<std::filesystem::path>& files) {
    if (files.empty()) {
        Logger::Log(Logger::ERROR, "Audio", "AudioClipLoader called with no source file.");
        return nullptr;
    }
    // One clip, one file. Extra sources would be a multi-variant clip, which the asset model does
    // not express yet; ignoring them silently would hide an import mistake.
    if (files.size() > 1) {
        Logger::Log(Logger::WARNING, "Audio", "AudioClipLoader got %zu sources for one clip; using '%s' and ignoring the rest.",
                    files.size(), files.front().string().c_str());
    }

    const auto& file = files.front();

    // Large files stream instead of decoding into RAM: a few minutes of music decodes to tens of
    // megabytes of PCM, which is not worth resident memory for something played once at a time.
    // The threshold is on the ENCODED size, since that is knowable without decoding first.
    std::error_code size_error;
    const auto encoded_size = std::filesystem::file_size(file, size_error);
    if (!size_error && encoded_size >= AudioClipLoader::streamingThresholdBytes()) {
        auto stream = OpenAudioFileStream(file);
        if (stream != nullptr) {
            auto clip = std::make_shared<AudioClip>(
                AudioClip::Streaming(stream->getChannels(), stream->getSampleRate(), stream->getTotalFrames()));
            clip->setSources(files);
            Logger::Log(Logger::DEBUG, "Audio", "Streaming '%s': %u ch @ %u Hz, %.2fs (%.1f MiB encoded)", file.string().c_str(),
                        clip->getChannels(), clip->getSampleRate(), clip->getDuration(), encoded_size / (1024.0 * 1024.0));
            return clip;
        }
        // Falling through to a full decode is the right failure mode: the file may still be
        // decodable in one shot even if the streaming path could not open it.
        Logger::Log(Logger::WARNING, "Audio", "Could not open '%s' for streaming; decoding it fully instead.", file.string().c_str());
    }

    auto decoded = DecodeAudioFile(file);
    if (!decoded.has_value()) {
        Logger::Log(Logger::ERROR, "Audio", "Could not decode audio file '%s' (unsupported format or corrupt data).",
                    file.string().c_str());
        return nullptr;
    }

    auto clip = std::make_shared<AudioClip>(std::move(decoded->samples), decoded->channels, decoded->sampleRate);
    clip->setSources(files);

    Logger::Log(Logger::DEBUG, "Audio", "Loaded '%s': %u ch @ %u Hz, %.2fs", file.string().c_str(), clip->getChannels(),
                clip->getSampleRate(), clip->getDuration());
    return clip;
}

}  // namespace ICE
