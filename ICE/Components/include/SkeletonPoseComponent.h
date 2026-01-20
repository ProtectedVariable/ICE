#pragma once

#include <Component.h> 

namespace ICE {
struct SkeletonPoseComponent : public Component {
    AssetUID skeletonModel = NO_ASSET_ID;
    std::vector<Eigen::Matrix4f> final_bone_matrices; 
};
}  // namespace ICE