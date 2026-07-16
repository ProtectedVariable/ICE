//
// Created by Thomas Ibanez on 31.07.21.
//

#pragma once

#include <assimp/material.h>
#include <assimp/scene.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Asset.h"
#include "AssetPath.h"
#include "IAssetLoader.h"
#include "Material.h"
#include "Model.h"
#include "Texture.h"

namespace ICE {
class AssetBank;

// A texture parsed off-thread (during stage()) together with the material-uniform slots it feeds.
// Its AssetUID is assigned only at commit(), once it is inserted into the bank.
struct StagedTexture {
    AssetPath path;
    std::shared_ptr<Texture2D> texture;
    std::string has_uniform;  // e.g. "material.hasAoMap"
    std::string map_uniform;  // e.g. "material.aoMap"
};

// A material parsed off-thread. Base uniforms and the shader are already set; the texture-map
// uniforms are wired up at commit() once the textures have UIDs.
struct StagedMaterial {
    AssetPath path;
    std::shared_ptr<Material> material;
    std::vector<StagedTexture> textures;
};

struct StagedMesh {
    AssetPath path;
    std::shared_ptr<Mesh> mesh;
};

// The full result of parsing a model file with no AssetBank access -- everything commit() needs to
// populate the bank on the main thread. `meshes[i]` and `materials[i]` correspond to the same scene
// mesh and are committed in that interleaved order so UID assignment matches the original loader.
struct StagedModel {
    std::vector<StagedMesh> meshes;
    std::vector<StagedMaterial> materials;
    std::vector<Model::Node> nodes;
    Model::Skeleton skeleton;
    std::unordered_map<std::string, Animation> animations;
    bool hasAnimations = false;
    std::vector<std::filesystem::path> sources;
    bool valid = false;  // false when the file was empty or the parse failed -> commit() does nothing
};

class ModelLoader : public IAssetLoader<Model> {
   public:
    ModelLoader(AssetBank &bank) : ref_bank(bank) {}

    // IAssetLoader contract: synchronous stage + commit on the calling thread. Behavior (and bank
    // contents) are identical to the previous single-pass loader.
    std::shared_ptr<Model> load(const std::vector<std::filesystem::path> &file) override;

    // Pure parse step (Assimp parse + texture decode): no AssetBank access, safe to run on a worker
    // thread. `pbr_shader_uid` is the one bank value the parse needs; the caller resolves it on the
    // main thread beforehand and passes it in.
    StagedModel stage(const std::vector<std::filesystem::path> &file, AssetUID pbr_shader_uid);

    // Bank mutation step: assigns UIDs (preserving them across re-import), wires material textures,
    // and builds the Model. Must run on the thread that owns `bank`. Returns nullptr, and touches
    // nothing, for an invalid StagedModel (empty file / failed parse).
    std::shared_ptr<Model> commit(StagedModel &staged, AssetBank &bank);

    int processNode(const aiNode *node, std::vector<Model::Node> &nodes, Model::Skeleton &skeleton, std::unordered_set<std::string> &used_names,
                    const Eigen::Matrix4f &parent_transform);
    std::unordered_map<int, Eigen::Matrix4f> extractBoneData(const aiMesh *mesh, MeshData &data, Model::Skeleton &skeleton);
    std::unordered_map<std::string, Animation> extractAnimations(const aiScene *scene, Model::Skeleton &skeleton);

   private:
    // stage helpers -- pure, no bank access.
    StagedMesh stageMesh(const aiMesh *mesh, const std::string &model_name, const aiScene *scene, Model::Skeleton &skeleton);
    StagedMaterial stageMaterial(const aiMaterial *material, const std::string &model_name, const aiScene *scene, AssetUID pbr_shader_uid);
    void stageTexture(StagedMaterial &staged_material, const aiMaterial *material, const std::string &tex_path, const aiScene *scene,
                      aiTextureType type, const std::string &has_uniform, const std::string &map_uniform);

    // commit helpers -- assign UIDs / write the bank (re-import preserves the existing UID).
    AssetUID commitMesh(StagedMesh &staged, AssetBank &bank);
    AssetUID commitMaterial(StagedMaterial &staged, AssetBank &bank);
    AssetUID commitTexture(StagedTexture &staged, AssetBank &bank);

    Eigen::Vector4f colorToVec(aiColor4D *color);
    Eigen::Matrix4f aiMat4ToEigen(const aiMatrix4x4 &mat);
    Eigen::Vector3f aiVec3ToEigen(const aiVector3D &vec);
    Eigen::Quaternionf aiQuatToEigen(const aiQuaternion &q);

    TextureFormat getTextureFormat(aiTextureType type, int channels);

    AssetBank &ref_bank;
};
}  // namespace ICE
