#pragma once

#include <AssetBank.h>
#include <JobScheduler.h>
#include <Model.h>  // Model::Node / Model::Skeleton are used by value here (nested types need the full type)
#include <Registry.h>
#include <SkeletonPoseComponent.h>

#include "Animation.h"
#include "AnimationComponent.h"
#include "System.h"

namespace ICE {

struct BonePose {
    Eigen::Vector3f position = Eigen::Vector3f::Zero();
    Eigen::Quaternionf rotation = Eigen::Quaternionf::Identity();
    Eigen::Vector3f scale = Eigen::Vector3f::Ones();
};

class AnimationSystem : public System {
   public:
    AnimationSystem(const std::shared_ptr<Registry> &reg, const std::shared_ptr<AssetBank> &bank);
    void update(double delta) override;

    int updateOrder() const override { return AnimationSystemOrder; }

    // Opt-in parallel skeleton update. With a scheduler set, each animated entity is sampled and
    // posed on a worker thread (asset access stays on the render thread; see update()). Null (the
    // default) keeps the single-threaded path.
    void setScheduler(const std::shared_ptr<JobScheduler> &scheduler) { m_scheduler = scheduler; }

    std::vector<Signature> getSignatures(const ComponentManager &comp_manager) const override {
        Signature signature;
        signature.set(comp_manager.getComponentType<AnimationComponent>());
        signature.set(comp_manager.getComponentType<SkeletonPoseComponent>());
        return {signature};
    }

   private:
    template<typename T>
    size_t findKeyIndex(double animationTime, const std::vector<T> &keys) {
        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (animationTime < keys[i + 1].timeStamp) {
                return i;
            }
        }
        return keys.size() - 1;
    }

    // Advance and pose one animated entity. Takes the already-resolved model so the parallel path
    // performs no asset-bank lookups (only the entity's own components + the shared, read-only
    // model). Safe to run concurrently across entities: each animated skeleton owns disjoint bone
    // entities, so the TransformComponent/pose writes never overlap.
    void updateEntity(Entity e, const std::shared_ptr<Model> &model, double dt);

    void updateSkeleton(const std::shared_ptr<Model> &model, double time, SkeletonPoseComponent *pose, const Animation &anim);
    void finalizePose(Entity e, const std::shared_ptr<Model> &model);

    BonePose sampleBonePose(const std::string &boneName, const Animation &anim, double time, const std::shared_ptr<Model> &model);
    static BonePose blendPoses(const BonePose &a, const BonePose &b, float factor);

    Eigen::Vector3f interpolatePosition(double timeInTicks, const BoneAnimation &track);
    Eigen::Vector3f interpolateScale(double timeInTicks, const BoneAnimation &track);
    Eigen::Quaternionf interpolateRotation(double time, const BoneAnimation &track);

    void applyTransforms(const Model::Node *node, const Eigen::Matrix4f &parentTransform, const Model::Skeleton &skeleton, double time,
                         SkeletonPoseComponent *pose, const Animation &anim, const std::vector<Model::Node> &allModelNodes);

    // Non-owning back-reference (the Registry owns this system) to avoid an ownership cycle.
    Registry* m_registry = nullptr;
    std::shared_ptr<AssetBank> m_asset_bank;

    // Optional work-stealing scheduler for the parallel skeleton-update path (null => serial).
    std::shared_ptr<JobScheduler> m_scheduler;
};
}  // namespace ICE
