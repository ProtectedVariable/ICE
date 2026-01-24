//
// Created by Thomas Ibanez on 29.07.21.
//

#pragma once

#include <ICEException.h>

#include <typeindex>
#include <unordered_map>
#include <variant>

#include "Asset.h"
#include "IAssetLoader.h"
#include "Model.h"

namespace ICE {
class AssetLoader {
   public:
    template<typename T>
    std::shared_ptr<T> LoadResource(const std::vector<std::filesystem::path> files) {
        if (loaders.find(typeid(T)) != loaders.end()) {
            auto &variant = loaders[typeid(T)];
            auto loader = std::get<std::shared_ptr<IAssetLoader<T>>>(variant);
            return loader->load(files);
        }
        throw ICEException("No matching loader for resource");
    }

    template<typename T>
    void AddLoader(std::shared_ptr<IAssetLoader<T>> loader) {
        loaders[typeid(T)] = loader;
    }

   private:
    std::unordered_map<
        std::type_index,
        std::variant<std::shared_ptr<IAssetLoader<Mesh>>, std::shared_ptr<IAssetLoader<Model>>, std::shared_ptr<IAssetLoader<Material>>,
                     std::shared_ptr<IAssetLoader<Shader>>, std::shared_ptr<IAssetLoader<Texture2D>>, std::shared_ptr<IAssetLoader<TextureCube>>>>
        loaders;
};
}  // namespace ICE
