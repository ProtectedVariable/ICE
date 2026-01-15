//
// Created by Thomas Ibanez on 31.07.21.
//

#include "MeshLoader.h"

#include <AssetBank.h>
#include <BufferUtils.h>
#include <Material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

namespace ICE {
std::shared_ptr<Mesh> MeshLoader::load(const std::vector<std::filesystem::path> &file) {
    if (file.empty()) {
        return nullptr;
    }

    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(file[0].string(),
                                             aiProcess_FlipUVs | aiProcess_ValidateDataStructure | aiProcess_SortByPType | aiProcess_GenSmoothNormals
                                                 | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_PreTransformVertices);

    if (scene->mNumMeshes < 1) {
        return nullptr;
    }
    MeshData data;
    auto mesh = scene->mMeshes[0];
    for (int i = 0; i < mesh->mNumVertices; i++) {
        auto v = mesh->mVertices[i];
        auto n = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D{0, 0, 0};
        auto t = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i] : aiVector3D{0, 0, 0};
        auto b = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i] : aiVector3D{0, 0, 0};
        Eigen::Vector2f uv(0, 0);
        if (mesh->mTextureCoords[0] != nullptr) {
            auto uv_file = mesh->mTextureCoords[0][i];
            uv.x() = uv_file.x;
            uv.y() = uv_file.y;
        }
        data.vertices.emplace_back(v.x, v.y, v.z);
        data.normals.emplace_back(n.x, n.y, n.z);
        data.uvCoords.push_back(uv);
        data.tangents.emplace_back(t.x, t.y, t.z);
        data.bitangents.emplace_back(b.x, b.y, b.z);
        data.boneIDs.emplace_back(Eigen::Vector4i::Constant(-1));
        data.boneWeights.emplace_back(Eigen::Vector4f::Zero());
    }
    for (int i = 0; i < mesh->mNumFaces; i++) {
        auto f = mesh->mFaces[i];
        assert(f.mNumIndices == 3);
        data.indices.emplace_back(f.mIndices[0], f.mIndices[1], f.mIndices[2]);
    }

    auto mesh_ = std::make_shared<Mesh>(data);
    mesh_->setSources(file);
    return mesh_;
}

}  // namespace ICE
