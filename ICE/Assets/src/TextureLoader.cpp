//
// Created by Thomas Ibanez on 29.07.21.
//

#include <Graphics/Texture.h>
#include "TextureLoader.h"

namespace ICE {
    Resource *Texture2DLoader::load(const std::vector<std::string> &file) {
        Texture* texture = Texture2D::Create(file[0]);
        return new Resource(texture, file);
    }

    Resource *TextureCubeLoader::load(const std::vector<std::string> &file) {
        Texture* texture = TextureCube::Create(file[0]);
        return new Resource(texture, file);
    }
}