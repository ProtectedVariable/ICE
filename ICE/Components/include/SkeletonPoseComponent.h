#pragma once

#include <Asset.h>
#include <Component.h>

#include <Eigen/Dense>
#include <string>
#include <unordered_map>
#include <vector>

namespace ICE {
struct SkeletonPoseComponent : public Component {
    AssetUID skeletonModel = NO_ASSET_ID;
    std::vector<Eigen::Matrix4f> bone_transform; 
    std::unordered_map<std::string, Entity> bone_entity;
};
}  // namespace ICE