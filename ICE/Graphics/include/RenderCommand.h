//
// Created by Thomas Ibanez on 19.11.20.
//

#pragma once

#include <memory>
#include <unordered_map>

#include "GPUMesh.h"
#include "Material.h"
#include "Model.h"
#include "RenderState.h"
#include "ShaderProgram.h"

namespace ICE {

struct InstanceData;  // Forward declaration

struct RenderCommand {
    // mesh/shader are resolved from generational handles to raw pointers once, in
    // ForwardRenderer::prepareFrame; material is a CPU asset (raw pointer into the asset bank).
    GPUMesh* mesh = nullptr;
    Material* material = nullptr;
    ShaderProgram* shader = nullptr;

    // Textures are no longer carried per-command: the geometry pass resolves each of the
    // material's texture uniforms to a GPUTexture* via the registry at bind time.

    // Model matrix - direct value, no pointer
    Eigen::Matrix4f model_matrix;

    // Bone matrices - pointer to avoid copying large map
    const std::unordered_map<int, Eigen::Matrix4f>* bones = nullptr;

    // Render state - packed into bitfield to save space. Default-initialized so commands
    // (e.g. the skybox) that don't set them don't read indeterminate values.
    bool faceCulling : 1 = true;
    bool depthTest : 1 = true;
    bool depthWrite : 1 = true;
    bool is_instanced : 1 = false;
    // Alpha blending: only transparent materials need it; opaque draws leave it off so they
    // don't lose early-Z/HSR to a blend that does nothing.
    bool blend : 1 = false;

    DepthFunc depth_func = DepthFunc::Less;
    
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
        // Clamp instead of masking: depth_sq * 1000 overflowed 21 bits past ~46 units and
        // wrapped, scrambling the order. Clamped, far objects just pin at the max bucket.
        uint64_t depth_raw = static_cast<uint64_t>(depth_sq * 1000.0f);
        uint64_t depth_bits = depth_raw > 0x1FFFFF ? 0x1FFFFF : depth_raw;

        if (is_transparent) {
            // Alpha blending needs back-to-front: invert depth so farther fragments (larger
            // depth) get a smaller key and are drawn first. The old code sorted front-to-back.
            uint64_t depth_far_first = 0x1FFFFF - depth_bits;
            sort_key = (transparent_bit << 63) | (depth_far_first << 42) | (shader_bits << 21) | material_bits;
        } else {
            // Opaque: sort by shader/material to minimize state changes, then front-to-back.
            sort_key = (transparent_bit << 63) | (shader_bits << 42) | (material_bits << 21) | depth_bits;
        }
    }
};

inline bool operator<(const RenderCommand& a, const RenderCommand& b) {
    return a.sort_key < b.sort_key;
}
}  // namespace ICE

