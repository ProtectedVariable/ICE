#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace ICE {

// Decoded PCM, in the one format every OpenAL implementation accepts without extensions:
// signed 16-bit, interleaved. Float output would need AL_EXT_FLOAT32, so the conversion is done
// here, once, at decode time rather than being pushed onto the backend.
struct DecodedAudio {
    std::vector<int16_t> samples;  // interleaved, channels * frameCount entries
    uint32_t channels = 0;
    uint32_t sampleRate = 0;
    uint64_t frameCount = 0;

    double duration() const { return sampleRate == 0 ? 0.0 : static_cast<double>(frameCount) / sampleRate; }
};

// Decode a WAV / MP3 / FLAC / OGG file to interleaved 16-bit PCM, dispatching on the file
// extension. Returns nullopt if the extension is unknown or the decode fails; the caller logs.
//
// This is a blocking, CPU-bound whole-file decode -- it is what AssetBank::requestAsset stages on
// the JobScheduler off the main thread. Streaming (incremental decode for music) is separate and
// does not go through here.
std::optional<DecodedAudio> DecodeAudioFile(const std::filesystem::path& file);

// Extensions DecodeAudioFile understands, lowercase and dot-prefixed (".wav", ...). Used by the
// asset browser to filter importable files.
const std::vector<std::string>& SupportedAudioExtensions();

// Average multi-channel audio down to one channel in place. Needed because OpenAL positions MONO
// buffers only -- a stereo buffer is played flat at full volume, ignoring the listener entirely.
// A clip intended for 3D playback therefore has to be mono before it reaches the device, and doing
// it here (once, at import) beats discovering it as a "3D audio doesn't work" bug later.
// No-op if the audio is already mono.
void DownmixToMono(DecodedAudio& audio);

}  // namespace ICE
