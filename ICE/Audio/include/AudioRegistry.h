#pragma once

#include <AssetBank.h>
#include <AudioClip.h>

#include <memory>
#include <unordered_map>

#include "IAudioBackend.h"

namespace ICE {
class IAudioStream;

// Owns the backend-resident audio buffers uploaded from AudioClip assets, keyed by AssetUID. The
// direct counterpart of GPURegistry: assets stay CPU-only, the device-side object lives here, and
// eviction is driven by the AssetBank removal listener so a re-imported clip drops its stale
// upload without `assets` needing to know the audio layer exists.
//
// Uploads are lazy: the first getBuffer() for a UID uploads, subsequent ones hit the map.
class AudioRegistry {
   public:
    AudioRegistry(const std::shared_ptr<IAudioBackend>& backend, const std::shared_ptr<AssetBank>& bank);
    ~AudioRegistry();

    // Single owner of the buffer map and holder of a self-referential listener registration:
    // copying would double-unregister and not own its own listener.
    AudioRegistry(const AudioRegistry&) = delete;
    AudioRegistry& operator=(const AudioRegistry&) = delete;

    // Upload-on-first-use. Returns a null handle if the UID is unknown, is not an AudioClip, is
    // still loading (async import), or failed to decode.
    AudioBufferHandle getBuffer(AssetUID clip);

    // The CPU-side clip behind a UID, or nullptr if it is unknown, of another type, or still
    // loading. Callers need this for format questions the buffer handle cannot answer -- above all
    // whether the clip is mono, which decides if it can be spatialized at all.
    std::shared_ptr<AudioClip> getClip(AssetUID clip) const;

    // Open a fresh decoder over a streaming clip's source file. Each call returns an INDEPENDENT
    // stream with its own read position, so the same music can play twice at once (a crossfade
    // between a track and itself, for instance). Null unless the clip exists, is streaming, and
    // its source file can be opened.
    std::shared_ptr<IAudioStream> openStream(AssetUID clip) const;

    // Release the buffer uploaded from `clip`, if any. Invoked via the AssetBank removal listener;
    // safe to call for a UID that was never uploaded.
    void evict(AssetUID clip);

    // Drop every uploaded buffer (used at shutdown, before the backend goes away).
    void clear();

    std::size_t residentBufferCount() const { return m_buffers.size(); }

   private:
    std::shared_ptr<IAudioBackend> m_backend;
    std::shared_ptr<AssetBank> m_asset_bank;
    std::unordered_map<AssetUID, AudioBufferHandle> m_buffers;
    AssetBank::RemovalListenerHandle m_listener = 0;
};

}  // namespace ICE
