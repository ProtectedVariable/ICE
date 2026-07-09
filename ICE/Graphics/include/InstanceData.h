//
// Created for ICE rendering improvements
//

#pragma once

#include <Eigen/Dense>

namespace ICE {

// Per-instance data for instanced rendering.
struct InstanceData {
    Eigen::Matrix4f model_matrix;
    // Future: Add per-instance color, material index, etc.
};

}  // namespace ICE
