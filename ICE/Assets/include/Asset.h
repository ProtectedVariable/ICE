//
// Created by Thomas Ibanez on 16.07.21.
//

#ifndef ICE_ASSET_H
#define ICE_ASSET_H

#define NO_ASSET_ID ((unsigned long long)0)

#include <string>

namespace ICE {

    typedef unsigned long long AssetUID;

    enum class AssetType {
        ETex2D, ETexCube, EShader, EMesh, EMaterial
    };

    class Asset {
    public:
        static AssetType type;
        virtual std::string getTypeName() = 0;
    };
}
#endif //ICE_ASSET_H
