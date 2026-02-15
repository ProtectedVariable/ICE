//
// Created for ICE rendering improvements
//

#pragma once

#include <Eigen/Dense>
#include <memory>
#include <vector>
#include <unordered_map>

#include "GPUMesh.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "Buffers.h"

namespace ICE {

// Per-instance data for instanced rendering
struct InstanceData {
    Eigen::Matrix4f model_matrix;
    // Future: Add per-instance color, material index, etc.
};

// Batch of instances sharing the same mesh/material/shader
struct InstanceBatch {
    std::shared_ptr<GPUMesh> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<ShaderProgram> shader;
    std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> textures;
    std::vector<InstanceData> instances;
    
    // GPU buffer for instance data (created on demand)
    std::shared_ptr<VertexBuffer> instance_buffer;
    bool buffer_dirty = true;
    
    // Generate hash key for batching
    uint64_t getBatchKey() const {
        uint64_t hash = 0;
        hash ^= reinterpret_cast<uint64_t>(mesh.get()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= reinterpret_cast<uint64_t>(material.get()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= reinterpret_cast<uint64_t>(shader.get()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};

}  // namespace ICE
