//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <assimp/material.h>
#include <assimp/scene.h>

#include <string>

#include "Asset.h"
#include "IAssetLoader.h"
#include "Mesh.h"

namespace ICE {
class AssetBank;

class MeshLoader : public IAssetLoader<Mesh> {
   public:
    MeshLoader() {}

    std::shared_ptr<Mesh> load(const std::vector<std::filesystem::path> &file) override;
};
}  // namespace ICE
