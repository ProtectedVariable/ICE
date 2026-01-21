#include "AnimationSystem.h"

#include <iostream>

namespace ICE {
AnimationSystem::AnimationSystem(const std::shared_ptr<Registry>& reg, const std::shared_ptr<AssetBank>& bank) : m_registry(reg), m_asset_bank(bank) {
}

void AnimationSystem::update(double dt) {
    for (auto e : entities) {
        auto anim = m_registry->getComponent<AnimationComponent>(e);
        auto pose = m_registry->getComponent<SkeletonPoseComponent>(e);
        if (!anim->playing)
            continue;

        anim->currentTime += dt * anim->speed;

        auto model = m_asset_bank->getAsset<Model>(pose->skeletonModel);
        if (!model->getAnimations().contains(anim->currentAnimation)) {
            continue;
        }
        auto animation = model->getAnimations().at(anim->currentAnimation);

        if (anim->currentTime > animation.duration) {
            if (anim->loop) {
                anim->currentTime = std::fmod(anim->currentTime, animation.duration);
            } else {
                anim->currentTime = animation.duration;
                anim->playing = false;
            }
        }
        updateSkeleton(model, anim->currentTime, pose, animation);
        finalizePose();
    }
}

void AnimationSystem::updateSkeleton(const std::shared_ptr<Model>& model, double time, SkeletonPoseComponent* pose, const Animation& anim) {
    for (auto const& [nodeName, nodeEntity] : pose->bone_entity) {
        if (anim.tracks.contains(nodeName)) {
            auto transform = m_registry->getComponent<TransformComponent>(nodeEntity);
            const auto& track = anim.tracks.at(nodeName);

            auto pos = interpolatePosition(time, track);
            auto rot = interpolateRotation(time, track);
            auto scale = interpolateScale(time, track);

            transform->setPosition(pos);
            transform->setRotation(rot);
            transform->setScale(scale);
        }
    }
}

void AnimationSystem::finalizePose() {
    for (auto e : entities) {
        auto pose = m_registry->getComponent<SkeletonPoseComponent>(e);
        auto model = m_asset_bank->getAsset<Model>(pose->skeletonModel);
        auto& skeleton = model->getSkeleton();

        auto rootTransform = m_registry->getComponent<TransformComponent>(e);
        Eigen::Matrix4f modelWorldInv = rootTransform->getWorldMatrix().inverse();

        for (const auto& [name, id] : skeleton.boneMapping) {
            Entity boneEntity = pose->bone_entity.at(name);

            Eigen::Matrix4f boneWorld = m_registry->getComponent<TransformComponent>(boneEntity)->getWorldMatrix();
            pose->bone_transform[id] = boneWorld;
        }
    }
}

Eigen::Vector3f AnimationSystem::interpolatePosition(double timeInTicks, const BoneAnimation& track) {
    if (track.positions.empty()) {
        return Eigen::Vector3f::Zero();
    }
    if (track.positions.size() == 1) {
        return track.positions[0].position;
    }

    size_t startIndex = findKeyIndex(timeInTicks, track.positions);
    size_t nextIndex = std::min(startIndex + 1, track.positions.size() - 1);

    const auto& startKey = track.positions[startIndex];
    const auto& nextKey = track.positions[nextIndex];

    double totalTime = nextKey.timeStamp - startKey.timeStamp;
    if (totalTime == 0.0)
        return Eigen::Vector3f::Zero();

    double currentTime = timeInTicks - startKey.timeStamp;
    float factor = (float) (currentTime / totalTime);

    Eigen::Vector3f interpolatedPosition = startKey.position + factor * (nextKey.position - startKey.position);

    return interpolatedPosition;
}

Eigen::Vector3f AnimationSystem::interpolateScale(double timeInTicks, const BoneAnimation& track) {
    if (track.scales.empty()) {
        return Eigen::Vector3f::Ones();
    }
    if (track.scales.size() == 1) {
        return track.scales[0].scale;
    }

    size_t startIndex = findKeyIndex(timeInTicks, track.scales);
    size_t nextIndex = std::min(startIndex + 1, track.scales.size() - 1);

    const auto& startKey = track.scales[startIndex];
    const auto& nextKey = track.scales[nextIndex];

    double totalTime = nextKey.timeStamp - startKey.timeStamp;
    if (totalTime == 0.0)
        return Eigen::Vector3f::Ones();

    double currentTime = timeInTicks - startKey.timeStamp;
    float factor = (float) (currentTime / totalTime);

    Eigen::Vector3f interpolatedScale = startKey.scale + factor * (nextKey.scale - startKey.scale);

    return interpolatedScale;
}

Eigen::Quaternionf AnimationSystem::interpolateRotation(double time, const BoneAnimation& track) {
    if (track.rotations.size() == 1) {
        return track.rotations[0].rotation;
    }

    size_t startIndex = findKeyIndex(time, track.rotations);
    size_t nextIndex = std::min(startIndex + 1, track.rotations.size() - 1);

    const auto& startKey = track.rotations[startIndex];
    const auto& nextKey = track.rotations[nextIndex];

    double totalTime = nextKey.timeStamp - startKey.timeStamp;
    double factor = (time - startKey.timeStamp) / totalTime;

    Eigen::Quaternionf finalQuat = startKey.rotation.slerp((float) factor, nextKey.rotation);
    finalQuat.normalize();

    return finalQuat;
}

}  // namespace ICE