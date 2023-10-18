//
// Created by Thomas Ibanez on 29.07.21.
//

#ifndef ICE_TEXTURELOADER_H
#define ICE_TEXTURELOADER_H


#include <string>
#include "ResourceLoader.h"
#include "Asset.h"

namespace ICE {
    class Texture2DLoader : public IResourceLoader {
    public:
        Resource *load(const std::vector<std::string> &file) override;
    };
    class TextureCubeLoader : public IResourceLoader {
    public:
        Resource *load(const std::vector<std::string> &file) override;
    };
}


#endif //ICE_TEXTURELOADER_H
