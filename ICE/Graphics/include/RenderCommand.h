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
    std::shared_ptr<GPUMesh> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<ShaderProgram> shader;
    std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> textures;
    Eigen::Matrix4f model_matrix;

    std::unordered_map<int, Eigen::Matrix4f> bones;

    bool faceCulling = true;
    bool depthTest = true;
    
    // Instancing support
    bool is_instanced = false;
    const std::vector<InstanceData>* instance_data = nullptr;
    uint32_t instance_count = 1;
};
}  // namespace ICE

