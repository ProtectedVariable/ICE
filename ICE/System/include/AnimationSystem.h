#pragma once

#include <Registry.h>

#include "Animation.h"
#include "AnimationComponent.h"
#include "System.h"

namespace ICE {
class AnimationSystem : public System {
   public:
    void update(double delta) override;

   private:
    template<typename T>
    size_t findKeyIndex(double animationTime, const std::vector<T>& keys) {
        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (animationTime < keys[i + 1].timeStamp) {
                return i;
            }
        }
        return keys.size() - 1;
    }

    void updateSkeleton(const std::shared_ptr<Model>& model, double time, const Animation& anim);

    Eigen::Matrix4f interpolatePosition(double timeInTicks, const BoneAnimation& track);

    Eigen::Matrix4f interpolateScale(double timeInTicks, const BoneAnimation& track);

    Eigen::Matrix4f interpolateRotation(double time, const BoneAnimation& track);

    void applyTransforms(Model::Node* node, const Eigen::Matrix4f& parentTransform, Model::Skeleton& skeleton, double time, const Animation& anim,
                         std::vector<Model::Node>& allModelNodes);

    std::shared_ptr<Registry> m_registry;
    std::shared_ptr<AssetBank> m_asset_bank;
};
}  // namespace ICE
