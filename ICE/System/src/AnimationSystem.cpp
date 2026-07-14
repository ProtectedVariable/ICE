#include "AnimationSystem.h"

#include <iostream>
#include <TransformComponent.h>

namespace ICE {
AnimationSystem::AnimationSystem(const std::shared_ptr<Registry>& reg, const std::shared_ptr<AssetBank>& bank) : m_registry(reg.get()), m_asset_bank(bank) {
}

void AnimationSystem::update(double dt) {
    for (auto e : entities) {
        auto anim = m_registry->getComponent<AnimationComponent>(e);
        auto pose = m_registry->getComponent<SkeletonPoseComponent>(e);
        if (!anim->playing)
            continue;

        auto model = m_asset_bank->getAsset<Model>(pose->skeletonModel);
        const auto& animations = model->getAnimations();

        if (!animations.contains(anim->currentAnimation)) {
            continue;
        }
        const auto& currentAnim = animations.at(anim->currentAnimation);

        // Advance current animation time. dt is in seconds; animation keyframes are in
        // ticks, so convert with ticksPerSecond (previously ignored -> wrong playback rate).
        anim->currentTime += dt * currentAnim.ticksPerSecond * anim->speed;

        if (anim->currentTime > currentAnim.duration) {
            if (anim->loop) {
                anim->currentTime = std::fmod(anim->currentTime, currentAnim.duration);
            } else {
                anim->currentTime = currentAnim.duration;
                anim->playing = false;
            }
        }

        // Handle blending
        if (anim->blending) {
            anim->blendFactor += dt / anim->blendDuration;
            if (anim->blendFactor >= 1.0) {
                anim->blendFactor = 1.0;
                anim->blending = false;
            }

            // Advance previous animation time as well
            if (animations.contains(anim->previousAnimation)) {
                const auto& prevAnim = animations.at(anim->previousAnimation);
                anim->previousTime += dt * prevAnim.ticksPerSecond * anim->speed;
                if (anim->previousTime > prevAnim.duration) {
                    anim->previousTime = std::fmod(anim->previousTime, prevAnim.duration);
                }
            }

            // Blended update
            const Animation* prevAnimPtr = nullptr;
            if (animations.contains(anim->previousAnimation)) {
                prevAnimPtr = &animations.at(anim->previousAnimation);
            }

            float blendT = static_cast<float>(anim->blendFactor);

            for (auto const& [nodeName, nodeEntity] : pose->bone_entity) {
                BonePose currentPose = sampleBonePose(nodeName, currentAnim, anim->currentTime, model);
                BonePose prevPose;
                if (prevAnimPtr) {
                    prevPose = sampleBonePose(nodeName, *prevAnimPtr, anim->previousTime, model);
                } else {
                    prevPose = currentPose;
                }

                BonePose finalPose = blendPoses(prevPose, currentPose, blendT);

                auto transform = m_registry->getComponent<TransformComponent>(nodeEntity);
                transform->setPosition(finalPose.position);
                transform->setRotation(finalPose.rotation);
                transform->setScale(finalPose.scale);
            }
        } else {
            // No blending — direct update
            updateSkeleton(model, anim->currentTime, pose, currentAnim);
        }

        finalizePose(e);
    }
}

BonePose AnimationSystem::sampleBonePose(const std::string& boneName, const Animation& anim, double time, const std::shared_ptr<Model>& model) {
    BonePose pose;
    if (anim.tracks.contains(boneName)) {
        const auto& track = anim.tracks.at(boneName);
        pose.position = interpolatePosition(time, track);
        pose.rotation = interpolateRotation(time, track);
        pose.scale = interpolateScale(time, track);
    } else {
        // Fall back to default node transform
        const auto* node = model->getNodeByName(boneName);
        if (node) {
            TransformComponent defaultTransform(node->localTransform);
            pose.position = defaultTransform.getPosition();
            pose.rotation = defaultTransform.getRotation();
            pose.scale = defaultTransform.getScale();
        }
    }
    return pose;
}

BonePose AnimationSystem::blendPoses(const BonePose& a, const BonePose& b, float factor) {
    BonePose result;
    result.position = a.position + factor * (b.position - a.position);
    result.rotation = a.rotation.slerp(factor, b.rotation);
    result.rotation.normalize();
    result.scale = a.scale + factor * (b.scale - a.scale);
    return result;
}

void AnimationSystem::updateSkeleton(const std::shared_ptr<Model>& model, double time, SkeletonPoseComponent* pose, const Animation& anim) {
    for (auto const& [nodeName, nodeEntity] : pose->bone_entity) {
        BonePose bonePose = sampleBonePose(nodeName, anim, time, model);

        auto transform = m_registry->getComponent<TransformComponent>(nodeEntity);
        transform->setPosition(bonePose.position);
        transform->setRotation(bonePose.rotation);
        transform->setScale(bonePose.scale);
    }
}

void AnimationSystem::finalizePose(Entity e) {
    // Finalize only the entity being processed. This used to loop over every animated
    // entity on each call, and it is called once per entity in update(), so the work was
    // O(N^2) (with a matrix inverse per skeleton) while producing identical results.
    auto pose = m_registry->getComponent<SkeletonPoseComponent>(e);
    auto model = m_asset_bank->getAsset<Model>(pose->skeletonModel);
    auto& skeleton = model->getSkeleton();

    auto rootTransform = m_registry->getComponent<TransformComponent>(e);
    Eigen::Matrix4f modelWorldInv = rootTransform->getWorldMatrix().inverse();

    for (const auto& [name, id] : skeleton.boneMapping) {
        Entity boneEntity = pose->bone_entity.at(name);

        Eigen::Matrix4f boneWorld = m_registry->getComponent<TransformComponent>(boneEntity)->getWorldMatrix();
        pose->bone_transform[id] = modelWorldInv * boneWorld;
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
    // Empty track: nothing to interpolate. Without this guard, rotations.size() - 1 wraps
    // to SIZE_MAX and findKeyIndex reads out of bounds.
    if (track.rotations.empty()) {
        return Eigen::Quaternionf::Identity();
    }
    if (track.rotations.size() == 1) {
        return track.rotations[0].rotation;
    }

    size_t startIndex = findKeyIndex(time, track.rotations);
    size_t nextIndex = std::min(startIndex + 1, track.rotations.size() - 1);

    const auto& startKey = track.rotations[startIndex];
    const auto& nextKey = track.rotations[nextIndex];

    double totalTime = nextKey.timeStamp - startKey.timeStamp;
    if (totalTime == 0.0) {
        return startKey.rotation;
    }
    double factor = (time - startKey.timeStamp) / totalTime;

    Eigen::Quaternionf finalQuat = startKey.rotation.slerp((float) factor, nextKey.rotation);
    finalQuat.normalize();

    return finalQuat;
}

}  // namespace ICE