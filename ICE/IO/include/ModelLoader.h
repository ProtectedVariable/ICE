//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <string>

#include "Asset.h"
#include "IAssetLoader.h"
#include "Model.h"

namespace ICE {
class ModelLoader : public IAssetLoader<Model> {
   public:
    ModelLoader(const std::shared_ptr<GraphicsFactory> &factory) : IAssetLoader<Mesh>(factory) {}
    std::shared_ptr<Model> load(const std::vector<std::filesystem::path> &file) override;
};
}  // namespace ICE
