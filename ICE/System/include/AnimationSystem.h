#pragma once

#include <Registry.h>

#include "Animation.h"
#include "AnimationComponent.h"
#include "System.h"

namespace ICE {
class AnimationSystem : public System {
   public:
    AnimationSystem(const std::shared_ptr<Registry>& reg, const std::shared_ptr<AssetBank>& bank);
    void update(double delta) override;

    std::vector<Signature> getSignatures(const ComponentManager& comp_manager) const override {
        Signature signature;
        signature.set(comp_manager.getComponentType<AnimationComponent>());
        signature.set(comp_manager.getComponentType<SkeletonPoseComponent>());
        return {signature};
    }

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

    void updateSkeleton(const std::shared_ptr<Model>& model, double time, SkeletonPoseComponent* pose, const Animation& anim);
    void finalizePose();
    Eigen::Vector3f interpolatePosition(double timeInTicks, const BoneAnimation& track);

    Eigen::Vector3f interpolateScale(double timeInTicks, const BoneAnimation& track);

    Eigen::Quaternionf interpolateRotation(double time, const BoneAnimation& track);

    void applyTransforms(const Model::Node* node, const Eigen::Matrix4f& parentTransform, const Model::Skeleton& skeleton, double time,
                         SkeletonPoseComponent* pose, const Animation& anim, const std::vector<Model::Node>& allModelNodes);

    std::shared_ptr<Registry> m_registry;
    std::shared_ptr<AssetBank> m_asset_bank;
};
}  // namespace ICE
