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
    ShaderLoader() = default;
    std::shared_ptr<Shader> load(const std::vector<std::filesystem::path> &file) override;
    std::string readAndResolveIncludes(const std::filesystem::path &file);
    constexpr ShaderStage stageFromString(const std::string &str);
};
}  // namespace ICE
