//
// Created by Thomas Ibanez on 19.11.20.
//

#pragma once

#include <memory>
#include <unordered_map>

#include "GPUMesh.h"
#include "Material.h"
#include "Model.h"
#include "ShaderProgram.h"

namespace ICE {

struct InstanceData;  // Forward declaration

struct RenderCommand {
    GPUMesh* mesh = nullptr;
    Material* material = nullptr;
    ShaderProgram* shader = nullptr;
    
    // Texture map - still needs shared_ptr for lifetime management
    // TODO: Could use texture indices into a global texture array
    const std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>>* textures = nullptr;
    
    // Model matrix - direct value, no pointer
    Eigen::Matrix4f model_matrix;

    // Bone matrices - pointer to avoid copying large map
    const std::unordered_map<int, Eigen::Matrix4f>* bones = nullptr;

    // Render state - packed into bitfield to save space. Default-initialized so commands
    // (e.g. the skybox) that don't set them don't read indeterminate values.
    bool faceCulling : 1 = true;
    bool depthTest : 1 = true;
    bool is_instanced : 1 = false;
    
    // Instancing support
    const std::vector<InstanceData>* instance_data = nullptr;
    uint32_t instance_count = 1;
    
    // Sorting key - pre-computed for fast sorting
    uint64_t sort_key = 0;
    
    // Helper to compute sort key
    void computeSortKey(bool is_transparent, float depth_sq) {
        // Pack: [transparent:1][shader:21][material:21][depth:21]
        uint64_t transparent_bit = is_transparent ? 1ULL : 0ULL;
        uint64_t shader_bits = (reinterpret_cast<uintptr_t>(shader) >> 3) & 0x1FFFFF;
        uint64_t material_bits = (reinterpret_cast<uintptr_t>(material) >> 3) & 0x1FFFFF;
        uint64_t depth_bits = static_cast<uint64_t>(depth_sq * 1000.0f) & 0x1FFFFF;
        
        if (is_transparent) {
            // Transparent: sort by depth (back-to-front)
            sort_key = (transparent_bit << 63) | (depth_bits << 42) | (shader_bits << 21) | material_bits;
        } else {
            // Opaque: sort by shader/material, then depth (front-to-back)
            sort_key = (transparent_bit << 63) | (shader_bits << 42) | (material_bits << 21) | depth_bits;
        }
    }
};

inline bool operator<(const RenderCommand& a, const RenderCommand& b) {
    return a.sort_key < b.sort_key;
}
}  // namespace ICE

