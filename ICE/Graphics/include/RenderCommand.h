//
// Created by Thomas Ibanez on 19.11.20.
//

#pragma once

#include <memory>
#include <unordered_map>

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "Model.h"

namespace ICE {
struct RenderCommand {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<Shader> shader;
    std::unordered_map<AssetUID, std::shared_ptr<Texture>> textures;
    Eigen::Matrix4f model_matrix;

    std::vector<Model::BoneInfo> bones;

    bool faceCulling = true;
    bool depthTest = true;
};
}  // namespace ICE
