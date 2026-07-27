#include "AudioClipLoader.h"

#include <Logger.h>

#include "AudioDecoder.h"

namespace ICE {

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
