//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <string>

#include "Asset.h"
#include "IAssetLoader.h"
#include "Mesh.h"

namespace ICE {
class MeshLoader : public IAssetLoader<Mesh> {
   public:
    MeshLoader(const std::shared_ptr<GraphicsFactory> &factory) : IAssetLoader<Mesh>(factory) {}
    std::shared_ptr<Mesh> load(const std::vector<std::filesystem::path> &file) override;
};
}  // namespace ICE
