#pragma once

#include <Component.h> 

namespace ICE {
struct SkinningComponent : public Component {
    std::vector<Entity> bone_entities;
    std::vector<Eigen::Matrix4f> inverse_bind_matrices;

};
}  // namespace ICE