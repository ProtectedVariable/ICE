#pragma once

#include <Asset.h>
#include <Component.h>

namespace ICE {
struct SkyboxComponent : public Component {
    SkyboxComponent(AssetUID texture_id) : texture(texture_id) {}
    AssetUID texture;
};
}  // namespace ICE