//
// Created by Thomas Ibanez on 29.11.20.
//

#include "AssetBank.h"

namespace ICE {

// Loaders are registered by the composition layer (io::registerDefaultLoaders),
// not here, so that the `assets` module does not depend on `io`.
AssetBank::AssetBank() = default;

bool AssetBank::nameInUse(const AssetPath &name) {
    return !(nameMapping.find(name) == nameMapping.end());
}
}  // namespace ICE