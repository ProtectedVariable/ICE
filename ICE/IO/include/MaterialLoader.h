//
// Created by Thomas Ibanez on 01.08.21.
//

#pragma once

#include <Material.h>

#include "IAssetLoader.h"

namespace ICE {
class MaterialLoader : public IAssetLoader<Material> {
   public:
    MaterialLoader() : IAssetLoader<Material>(nullptr) {}
    std::shared_ptr<Material> load(const std::vector<std::filesystem::path> &files) override;
};
}  // namespace ICE
