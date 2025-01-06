//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <assimp/material.h>
#include <assimp/scene.h>

#include <string>

#include "Asset.h"
#include "IAssetLoader.h"
#include "Model.h"

namespace ICE {
class AssetBank;

class ModelLoader : public IAssetLoader<Model> {
   public:
    ModelLoader(const std::shared_ptr<GraphicsFactory> &factory, AssetBank &bank)
        : ref_bank(bank),
          m_graphics_factory(factory),
          IAssetLoader<Model>(factory) {}
    std::shared_ptr<Model> load(const std::vector<std::filesystem::path> &file) override;
    AssetUID extractMaterial(const aiMaterial *material, const std::string &model_name, const aiScene *scene);

   private:
    Eigen::Vector4f colorToVec(aiColor4D *color);

    AssetBank &ref_bank;
    std::shared_ptr<GraphicsFactory> m_graphics_factory;
};
}  // namespace ICE
