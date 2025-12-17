//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <assimp/material.h>
#include <assimp/scene.h>

#include <string>

#include "Asset.h"
#include "IAssetLoader.h"
#include "Model.h"

namespace ICE {
class AssetBank;

class ModelLoader : public IAssetLoader<Model> {
   public:
    ModelLoader(const std::shared_ptr<GraphicsFactory> &factory, AssetBank &bank)
        : ref_bank(bank),
          m_graphics_factory(factory),
          IAssetLoader<Model>(factory) {}

    std::shared_ptr<Model> load(const std::vector<std::filesystem::path> &file) override;

    int processNode(const aiNode *node, std::vector<Model::Node> &nodes, Model::Skeleton &skeleton, std::unordered_set<std::string> &used_names,
                    const Eigen::Matrix4f &parent_transform);
    std::shared_ptr<Mesh> extractMesh(const aiMesh *mesh, const std::string &model_name, const aiScene *scene, Model::Skeleton &skeleton);
    AssetUID extractMaterial(const aiMaterial *material, const std::string &model_name, const aiScene *scene);
    AssetUID extractTexture(const aiMaterial *material, const std::string &tex_path, const aiScene *scene, aiTextureType type);
    void extractBoneData(const aiMesh *mesh, const aiScene *scene, MeshData &data, SkinningData &skinning_data, Model::Skeleton &skeleton);
    std::unordered_map<std::string, Animation> extractAnimations(const aiScene *scene, Model::Skeleton &skeleton);

   private:
    Eigen::Vector4f colorToVec(aiColor4D *color);
    Eigen::Matrix4f aiMat4ToEigen(const aiMatrix4x4 &mat);
    Eigen::Vector3f aiVec3ToEigen(const aiVector3D &vec);
    Eigen::Quaternionf aiQuatToEigen(const aiQuaternion &q);

    AssetBank &ref_bank;
    std::shared_ptr<GraphicsFactory> m_graphics_factory;
};
}  // namespace ICE
