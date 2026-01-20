#pragma once

#include <Component.h>

namespace ICE {
struct SkinningComponent : public Component {
    Entity skeleton_entity = 0;
};
}  // namespace ICE