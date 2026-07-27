#include "AudioRegistry.h"

#include <Logger.h>

namespace ICE {

AudioRegistry::AudioRegistry(const std::shared_ptr<IAudioBackend>& backend, const std::shared_ptr<AssetBank>& bank)
    : m_backend(backend),
      m_asset_bank(bank) {
    if (m_asset_bank != nullptr) {
        m_listener = m_asset_bank->addRemovalListener([this](AssetUID id) { evict(id); });
    }
}

AudioRegistry::~AudioRegistry() {
    if (m_asset_bank != nullptr && m_listener != 0) {
        m_asset_bank->removeRemovalListener(m_listener);
    }
    clear();
}

AudioBufferHandle AudioRegistry::getBuffer(AssetUID clip) {
    if (clip == NO_ASSET_ID || m_backend == nullptr || m_asset_bank == nullptr) {
        return {};
    }
    if (auto it = m_buffers.find(clip); it != m_buffers.end()) {
        return it->second;
    }

    // Null covers all of: unknown UID, wrong asset type, and an async import still in flight
    // (AssetBank::getAsset returns nullptr until the load is Ready). Callers just get no sound this
    // frame and retry next frame, which is the desired behaviour for a clip that is still loading.
    auto asset = m_asset_bank->getAsset<AudioClip>(clip);
    if (asset == nullptr || asset->isEmpty()) {
        return {};
    }

    AudioBufferHandle handle = m_backend->uploadClip(*asset);
    if (!handle.valid()) {
        Logger::Log(Logger::ERROR, "Audio", "Failed to upload audio clip %llu to the audio device.", (unsigned long long) clip);
        return {};
    }
    m_buffers.emplace(clip, handle);
    return handle;
}

std::shared_ptr<AudioClip> AudioRegistry::getClip(AssetUID clip) const {
    if (clip == NO_ASSET_ID || m_asset_bank == nullptr) {
        return nullptr;
    }
    return m_asset_bank->getAsset<AudioClip>(clip);
}

void AudioRegistry::evict(AssetUID clip) {
    auto it = m_buffers.find(clip);
    if (it == m_buffers.end()) {
        return;
    }
    m_backend->releaseBuffer(it->second);
    m_buffers.erase(it);
}

void AudioRegistry::clear() {
    if (m_backend != nullptr) {
        for (const auto& [uid, handle] : m_buffers) {
            m_backend->releaseBuffer(handle);
        }
    }
    m_buffers.clear();
}

}  // namespace ICE
