//
// Created by Thomas Ibanez on 16.07.21.
//

#pragma once

#define NO_ASSET_ID ((unsigned long long) 0)

#include <string>

#include "Resource.h"

namespace ICE {

typedef unsigned long long AssetUID;

enum class AssetType { ETex2D, ETexCube, EShader, EMesh, EMaterial };

class Asset : public Resource {
   public:
    Asset() : Resource({}) {}
    virtual AssetType getType() const = 0;
    virtual std::string getTypeName() const = 0;
    virtual void load() = 0;
    virtual void unload() = 0;
};
}  // namespace ICE
