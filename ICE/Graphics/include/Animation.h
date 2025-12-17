#pragma once

#include <Eigen/Dense>
#include <unordered_map>

namespace ICE {
struct BoneAnimation {
    struct PositionKey {
        float timeStamp;
        Eigen::Vector3f position;
    };
    struct RotationKey {
        float timeStamp;
        Eigen::Quaternionf rotation;
    };
    struct ScaleKey {
        float timeStamp;
        Eigen::Vector3f scale;
    };

    std::vector<PositionKey> positions;
    std::vector<RotationKey> rotations;
    std::vector<ScaleKey> scales;
};

struct Animation {
    float duration;
    float ticksPerSecond;

    std::unordered_map<std::string, BoneAnimation> tracks;
};
}