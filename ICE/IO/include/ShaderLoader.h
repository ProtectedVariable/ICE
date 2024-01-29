//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <string>

#include "IAssetLoader.h"
#include "Resource.h"
#include "Shader.h"

namespace ICE {
class ShaderLoader : public IAssetLoader<Shader> {
   public:
    ShaderLoader(const std::shared_ptr<GraphicsFactory> &factory) : IAssetLoader<Shader>(factory) {}
    std::shared_ptr<Shader> load(const std::vector<std::filesystem::path> &file) override;
};
}  // namespace ICE
