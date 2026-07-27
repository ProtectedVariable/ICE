#pragma once

#include <AudioClip.h>
#include <IAssetLoader.h>

namespace ICE {

// Decodes WAV/MP3/FLAC/OGG into an AudioClip. Lives in the `audio` module (not `io`, where the
// other loaders are) because the decoder libraries are a PRIVATE dependency of this module and
// must not leak into another module's include path. It is registered onto the bank from
// io/DefaultLoaders.cpp along with the rest.
//
// This is a pure loader -- it decodes and returns, without touching the bank -- so it works with
// AssetBank::requestAsset's convenience overload and gets background decoding on the JobScheduler
// for free.
class AudioClipLoader : public IAssetLoader<AudioClip> {
   public:
    // Returns nullptr on an unreadable file, an unsupported extension, or a decode failure. The
    // bank rejects a null result rather than inserting an unusable asset.
    std::shared_ptr<AudioClip> load(const std::vector<std::filesystem::path>& files) override;
};

}  // namespace ICE
