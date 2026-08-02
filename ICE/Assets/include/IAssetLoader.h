//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <filesystem>
#include <memory>
#include <type_traits>
#include <vector>

#include "Asset.h"
#include "GraphicsFactory.h"

namespace ICE {
// Non-template base so a heterogeneous set of loaders can live in a single container (AssetLoader)
// without that container having to know every concrete asset type at compile time. This is what
// lets `addLoader<T>` accept any T and keeps the assets headers from pulling in every leaf asset
// type.
class IAssetLoaderBase {
   public:
    virtual ~IAssetLoaderBase() = default;

    // Load `files` and return the result as the Asset base. Enables loading a type known only by
    // std::type_index (the async import queue and prefix-based project loading).
    virtual std::shared_ptr<Asset> loadErased(const std::vector<std::filesystem::path> &files) = 0;
};

template<typename T>
class IAssetLoader : public IAssetLoaderBase {
    static_assert(std::is_base_of_v<Asset, T>, "Asset loaders can only load types derived from Asset");

   public:
    IAssetLoader() = default;

    virtual std::shared_ptr<T> load(const std::vector<std::filesystem::path> &files) = 0;

    // Every loadable type is an Asset, so the typed result upcasts directly. A failed load returns
    // nullptr, which is preserved by the cast.
    std::shared_ptr<Asset> loadErased(const std::vector<std::filesystem::path> &files) override {
        return std::static_pointer_cast<Asset>(load(files));
    }
};
}  // namespace ICE
