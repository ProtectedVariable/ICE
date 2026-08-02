//
// Created by Thomas Ibanez on 29.07.21.
//

#pragma once

#include <ICEException.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "Asset.h"
#include "IAssetLoader.h"

namespace ICE {
// Open registry of asset loaders keyed by std::type_index. Loaders are stored type-erased behind
// IAssetLoaderBase, so registering a loader for a brand-new asset type needs no change here and this
// header no longer has to include every concrete asset type (Model/Material/Texture/...).
class AssetLoader {
   public:
    template<typename T>
    std::shared_ptr<T> LoadResource(const std::vector<std::filesystem::path> files) {
        // Downcast the erased result back to T. The stored loader is an IAssetLoader<T>, so this
        // always matches (or is nullptr on a failed load).
        return std::dynamic_pointer_cast<T>(LoadResource(std::type_index(typeid(T)), files));
    }

    // Erased entry point: load a type known only by its std::type_index. Used by the async import
    // queue and prefix-based project loading. Throws if no loader is registered for `type`.
    std::shared_ptr<Asset> LoadResource(std::type_index type, const std::vector<std::filesystem::path> &files) {
        if (auto it = loaders.find(type); it != loaders.end()) {
            return it->second->loadErased(files);
        }
        throw ICEException("No matching loader for resource");
    }

    template<typename T>
    void AddLoader(std::shared_ptr<IAssetLoader<T>> loader) {
        loaders[typeid(T)] = std::move(loader);
    }

    // The (type-erased) loader registered for `type`, or nullptr. Lets the async import path capture
    // the loader by shared_ptr and run it on a worker without touching the AssetBank.
    std::shared_ptr<IAssetLoaderBase> getLoader(std::type_index type) const {
        auto it = loaders.find(type);
        return it != loaders.end() ? it->second : nullptr;
    }

   private:
    std::unordered_map<std::type_index, std::shared_ptr<IAssetLoaderBase>> loaders;
};
}  // namespace ICE
