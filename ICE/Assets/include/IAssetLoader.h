//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <filesystem>

#include "Asset.h"
#include "GraphicsFactory.h"

namespace ICE {
template<typename T>
class IAssetLoader {
   public:
    IAssetLoader() = default;

    virtual std::shared_ptr<T> load(const std::vector<std::filesystem::path> &files) = 0;
};
}  // namespace ICE
