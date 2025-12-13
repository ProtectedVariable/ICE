#include "AnimationSystem.h"

#include <iostream>

namespace ICE {
AnimationSystem::AnimationSystem(const std::shared_ptr<Registry>& reg, const std::shared_ptr<AssetBank>& bank) : m_registry(reg), m_asset_bank(bank) {
}

void AnimationSystem::update(double dt) {
    for (auto e : entities) {
        auto anim = m_registry->getComponent<AnimationComponent>(e);
        auto rc = m_registry->getComponent<RenderComponent>(e);
        if (!anim->playing)
            continue;

        anim->currentTime += dt * anim->speed;

        auto model = m_asset_bank->getAsset<Model>(rc->model);
        auto animation = model->getAnimations().at(anim->currentAnimation);

        if (anim->currentTime > animation.duration) {
            if (anim->loop) {
                anim->currentTime = std::fmod(anim->currentTime, animation.duration);
            } else {
                anim->currentTime = animation.duration;
                anim->playing = false;
            }
        }
        updateSkeleton(model, anim->currentTime, animation);
    }
}

void AnimationSystem::updateSkeleton(const std::shared_ptr<Model>& model, double time, const Animation& anim) {
    auto& all_nodes = model->getNodes();
    applyTransforms(&all_nodes[0], Eigen::Matrix4f::Identity(), model->getSkeleton(), time, anim, all_nodes);
}

Eigen::Matrix4f AnimationSystem::interpolatePosition(double timeInTicks, const BoneAnimation& track) {
    if (track.positions.empty()) {
        return Eigen::Matrix4f::Identity();
    }
    if (track.positions.size() == 1) {
        Eigen::Matrix4f positionMatrix = Eigen::Matrix4f::Identity();
        positionMatrix.block<3, 1>(0, 3) = track.positions[0].position;
        return positionMatrix;
    }

    size_t startIndex = findKeyIndex(timeInTicks, track.positions);
    size_t nextIndex = startIndex + 1;

    const auto& startKey = track.positions[startIndex];
    const auto& nextKey = track.positions[nextIndex];

    double totalTime = nextKey.timeStamp - startKey.timeStamp;
    if (totalTime == 0.0)
        return Eigen::Matrix4f::Identity();

    double currentTime = timeInTicks - startKey.timeStamp;
    float factor = (float) (currentTime / totalTime);

    Eigen::Vector3f interpolatedPosition = startKey.position + factor * (nextKey.position - startKey.position);

    Eigen::Matrix4f positionMatrix = Eigen::Matrix4f::Identity();
    positionMatrix.block<3, 1>(0, 3) = interpolatedPosition;

    return positionMatrix;
}

Eigen::Matrix4f AnimationSystem::interpolateScale(double timeInTicks, const BoneAnimation& track) {
    if (track.scales.empty()) {
        return Eigen::Matrix4f::Identity();
    }
    if (track.scales.size() == 1) {
        Eigen::Matrix4f scaleMatrix = Eigen::Matrix4f::Identity();
        scaleMatrix.block<3, 3>(0, 0) = track.scales[0].scale.asDiagonal();
        return scaleMatrix;
    }

    size_t startIndex = findKeyIndex(timeInTicks, track.scales);
    size_t nextIndex = startIndex + 1;

    const auto& startKey = track.scales[startIndex];
    const auto& nextKey = track.scales[nextIndex];

    double totalTime = nextKey.timeStamp - startKey.timeStamp;
    if (totalTime == 0.0)
        return Eigen::Matrix4f::Identity();

    double currentTime = timeInTicks - startKey.timeStamp;
    float factor = (float) (currentTime / totalTime);

    Eigen::Vector3f interpolatedScale = startKey.scale + factor * (nextKey.scale - startKey.scale);

    Eigen::Matrix4f scaleMatrix = Eigen::Matrix4f::Identity();
    scaleMatrix.block<3, 3>(0, 0) = interpolatedScale.asDiagonal();
    return scaleMatrix;
}

Eigen::Matrix4f AnimationSystem::interpolateRotation(double time, const BoneAnimation& track) {
    if (track.rotations.size() == 1) {
        Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();
        rotation_matrix.block<3, 3>(0, 0) = track.rotations[0].rotation.toRotationMatrix();

        return rotation_matrix;
    }

    size_t startIdx = findKeyIndex(time, track.rotations);
    size_t nextIdx = startIdx + 1;

    const auto& startKey = track.rotations[startIdx];
    const auto& nextKey = track.rotations[nextIdx];

    double totalTime = nextKey.timeStamp - startKey.timeStamp;
    double factor = (time - startKey.timeStamp) / totalTime;

    Eigen::Quaternionf finalQuat = startKey.rotation.slerp((float) factor, nextKey.rotation);
    finalQuat.normalize();

    Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();
    rotation_matrix.block<3, 3>(0, 0) = finalQuat.toRotationMatrix();

    return rotation_matrix;
}

void AnimationSystem::applyTransforms(Model::Node* node, const Eigen::Matrix4f& parentTransform, Model::Skeleton& skeleton, double time,
                                      const Animation& anim, std::vector<Model::Node>& allModelNodes) {
    Eigen::Matrix4f nodeLocalTransform = node->localTransform;
    std::string nodeName = node->name;

    if (anim.tracks.contains(nodeName)) {
        const BoneAnimation& track = anim.tracks.at(nodeName);

        Eigen::Matrix4f posMatrix = interpolatePosition(time, track);
        Eigen::Matrix4f rotMatrix = interpolateRotation(time, track);
        Eigen::Matrix4f scaleMatrix = interpolateScale(time, track);

        float Sx = nodeLocalTransform.block<3, 1>(0, 0).norm();
        float Sy = nodeLocalTransform.block<3, 1>(0, 1).norm();
        float Sz = nodeLocalTransform.block<3, 1>(0, 2).norm();
        Eigen::Matrix4f originalScaleMatrix = Eigen::Matrix4f::Identity();
        originalScaleMatrix(0, 0) = Sx;
        originalScaleMatrix(1, 1) = Sy;
        originalScaleMatrix(2, 2) = Sz;

        nodeLocalTransform = originalScaleMatrix * posMatrix * rotMatrix * scaleMatrix;
    }

    Eigen::Matrix4f globalTransform = parentTransform * node->localTransform;
    node->animatedTransform = nodeLocalTransform;

    if (skeleton.boneMapping.contains(nodeName)) {
        int boneID = skeleton.boneMapping.at(nodeName);

        Eigen::Matrix4f finalMatrix = skeleton.globalInverseTransform * globalTransform * skeleton.bones[boneID].offsetMatrix;

        skeleton.bones[boneID].finalTransformation = finalMatrix;
    }

    for (int childIndex : node->children) {
        applyTransforms(&allModelNodes[childIndex], globalTransform, skeleton, time, anim, allModelNodes);
    }
}

}  // namespace ICE