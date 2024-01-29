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
    IAssetLoader(const std::shared_ptr<GraphicsFactory> &factory) : graphics_factory(factory) {}

    virtual std::shared_ptr<T> load(const std::vector<std::filesystem::path> &files) = 0;

   protected:
    std::shared_ptr<GraphicsFactory> graphics_factory;
};
}  // namespace ICE
