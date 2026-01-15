//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ModelLoader.h"

#include <AssetBank.h>
#include <BufferUtils.h>
#include <Material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <cassert>
#include <iostream>

namespace ICE {
std::shared_ptr<Model> ModelLoader::load(const std::vector<std::filesystem::path> &file) {
    Assimp::Importer importer;

    const aiScene *scene =
        importer.ReadFile(file[0].string(),
                          aiProcess_OptimizeGraph | aiProcess_FlipUVs | aiProcess_ValidateDataStructure | aiProcess_SortByPType
                              | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_LimitBoneWeights);

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<AssetUID> materials;
    std::vector<Model::Node> nodes;
    Model::Skeleton skeleton;
    skeleton.globalInverseTransform = aiMat4ToEigen(scene->mRootNode->mTransformation).inverse();
    for (int m = 0; m < scene->mNumMeshes; m++) {
        auto mesh = scene->mMeshes[m];
        auto material = scene->mMaterials[mesh->mMaterialIndex];
        auto model_name = file[0].filename().stem().string();
        meshes.push_back(extractMesh(mesh, model_name, scene, skeleton));
        materials.push_back(extractMaterial(material, model_name, scene));
        meshes.back()->setSources(file);
    }
    std::unordered_set<std::string> used_node_names;
    processNode(scene->mRootNode, nodes, skeleton, used_node_names, Eigen::Matrix4f::Identity());
    auto model = std::make_shared<Model>(nodes, meshes, materials);

    if (scene->HasAnimations()) {
        auto animations = extractAnimations(scene, skeleton);
        model->setAnimations(animations);
        model->setSkeleton(skeleton);
    }
    model->setSources(file);
    return model;
}

int ModelLoader::processNode(const aiNode *ainode, std::vector<Model::Node> &nodes, Model::Skeleton &skeleton,
                             std::unordered_set<std::string> &used_names, const Eigen::Matrix4f &parent_transform) {
    std::string name = ainode->mName.C_Str();
    if (used_names.contains(name)) {
        name = name + "_" + std::to_string(used_names.size());
    }
    used_names.insert(name);

    Model::Node node;
    node.name = name;

    aiMatrix4x4 local = ainode->mTransformation;
    node.localTransform = aiMat4ToEigen(local);
    node.animatedTransform = parent_transform * node.localTransform;

    for (unsigned int i = 0; i < ainode->mNumMeshes; ++i) {
        unsigned int mesh_idx = ainode->mMeshes[i];
        node.meshIndices.push_back(mesh_idx);
    }
    auto insert_pos = nodes.size();

    if (skeleton.boneMapping.contains(name)) {
        int boneID = skeleton.boneMapping.at(name);
        skeleton.bones[boneID].finalTransformation = skeleton.globalInverseTransform * node.animatedTransform;
    }

    nodes.push_back(node);

    for (unsigned int c = 0; c < ainode->mNumChildren; ++c) {
        const aiNode *child = ainode->mChildren[c];
        int child_pos = processNode(child, nodes, skeleton, used_names, node.animatedTransform);
        nodes.at(insert_pos).children.push_back(child_pos);
    }
    return insert_pos;
}

std::shared_ptr<Mesh> ModelLoader::extractMesh(const aiMesh *mesh, const std::string &model_name, const aiScene *scene, Model::Skeleton &skeleton) {
    MeshData data;

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

    SkinningData skinning_data;
    if (mesh->HasBones()) {
        extractBoneData(mesh, scene, data, skinning_data, skeleton);
    }

    auto mesh_ = std::make_shared<Mesh>(data);
    if (skinning_data.boneOffsetMatrices.size() > 0) {
        mesh_->setSkinningData(skinning_data);
    }

    return mesh_;
}

AssetUID ModelLoader::extractMaterial(const aiMaterial *material, const std::string &model_name, const aiScene *scene) {
    auto mtl_name = material->GetName();
    if (mtl_name.length == 0) {
        mtl_name = "DefaultMat";
    }
    auto bank_name = model_name + "/" + mtl_name.C_Str();
    auto mtl = std::make_shared<Material>();
    mtl->setUniform("material.hasAoMap", 0);
    mtl->setUniform("material.hasBaseColorMap", 0);
    mtl->setUniform("material.hasMetallicMap", 0);
    mtl->setUniform("material.hasRoughnessMap", 0);
    mtl->setUniform("material.hasNormalMap", 0);
    mtl->setUniform("material.hasEmissiveMap", 0);
    mtl->setUniform("material.ao", 1.0f);
    mtl->setUniform("material.metallic", 0.0f);
    mtl->setUniform("material.roughness", 1.0f);
    mtl->setShader(ref_bank.getUID(AssetPath::WithTypePrefix<Shader>("pbr")));
    // Base color
    aiColor4D diffuse = aiColor4D(1, 1, 1, 1);
    aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
    mtl->setUniform("material.baseColor", Eigen::Vector3f(colorToVec(&diffuse).head<3>()));

    ai_real roughness = 1.0f;
    aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &roughness);
    mtl->setUniform("material.roughness", (float) roughness);

    ai_real metallic = 0.0f;
    aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &metallic);
    mtl->setUniform("material.metallic", (float) metallic);

    if (auto ambient_map = extractTexture(material, bank_name + "/ao_map", scene, aiTextureType_LIGHTMAP); ambient_map != 0) {
        mtl->setUniform("material.hasAoMap", 1);
        mtl->setUniform("material.aoMap", ambient_map);
    }

    if (auto diffuse_tex = extractTexture(material, bank_name + "/diffuse_map", scene, aiTextureType_BASE_COLOR); diffuse_tex != 0) {
        mtl->setUniform("material.hasBaseColorMap", 1);
        mtl->setUniform("material.baseColorMap", diffuse_tex);
    }

    if (auto metallic_tex = extractTexture(material, bank_name + "/metallic_map", scene, aiTextureType_METALNESS); metallic_tex != 0) {
        mtl->setUniform("material.hasMetallicMap", 1);
        mtl->setUniform("material.metallicMap", metallic_tex);
    }

    if (auto roughness_tex = extractTexture(material, bank_name + "/roughness_map", scene, aiTextureType_DIFFUSE_ROUGHNESS); roughness_tex != 0) {
        mtl->setUniform("material.hasRoughnessMap", 1);
        mtl->setUniform("material.roughnessMap", roughness_tex);
    }

    if (auto normal_tex = extractTexture(material, bank_name + "/normal_map", scene, aiTextureType_NORMALS); normal_tex != 0) {
        mtl->setUniform("material.hasNormalMap", 1);
        mtl->setUniform("material.normalMap", normal_tex);
    }

    if (auto emissive_tex = extractTexture(material, bank_name + "/emissive_map", scene, aiTextureType_EMISSIVE); emissive_tex != 0) {
        mtl->setUniform("material.hasEmissiveMap", 1);
        mtl->setUniform("material.emissiveMap", emissive_tex);
    }

    if (ref_bank.getUID(AssetPath::WithTypePrefix<Material>(bank_name)) != 0) {
        return ref_bank.getUID(AssetPath::WithTypePrefix<Material>(bank_name));
    }

    ref_bank.addAsset<Material>(bank_name, mtl);
    return ref_bank.getUID(AssetPath::WithTypePrefix<Material>(bank_name));
}

AssetUID ModelLoader::extractTexture(const aiMaterial *material, const std::string &tex_path, const aiScene *scene, aiTextureType type) {
    AssetUID tex_id = 0;
    aiString texture_file;
    if (material->Get(AI_MATKEY_TEXTURE(type, 0), texture_file) == aiReturn_SUCCESS) {
        if (auto texture = scene->GetEmbeddedTexture(texture_file.C_Str())) {
            unsigned char *data = reinterpret_cast<unsigned char *>(texture->pcData);
            void *data2 = nullptr;
            int width = texture->mWidth;
            int height = texture->mHeight;
            int channels = 3;
            if (height == 0) {
                //Compressed memory, use stbi to load
                data2 = stbi_load_from_memory(data, texture->mWidth, &width, &height, &channels, 4);
                channels = 4;
            } else {
                data2 = data;
            }
            auto texture_ice = std::make_shared<Texture2D>(data2, width, height, getTextureFormat(type, channels));
            if (tex_id = ref_bank.getUID(AssetPath::WithTypePrefix<Texture2D>(tex_path)); tex_id != 0) {
                ref_bank.removeAsset(AssetPath::WithTypePrefix<Texture2D>(tex_path));
                ref_bank.addAssetWithSpecificUID(AssetPath::WithTypePrefix<Texture2D>(tex_path), texture_ice, tex_id);
            } else {
                ref_bank.addAsset<Texture2D>(tex_path, texture_ice);
                tex_id = ref_bank.getUID(AssetPath::WithTypePrefix<Texture2D>(tex_path));
            }
        } else {
            //regular file, check if it exists and read it
            //TODO :)
        }
    }
    return tex_id;
}

void ModelLoader::extractBoneData(const aiMesh *mesh, const aiScene *scene, MeshData &data, SkinningData &skinning_data, Model::Skeleton &skeleton) {
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        int boneID = -1;
        // If the bone is new (hasn't been added by a previous mesh)
        if (!skeleton.boneMapping.contains(boneName)) {
            boneID = skeleton.boneMapping.size();
            skeleton.boneMapping[boneName] = boneID;
            Model::BoneInfo newBoneInfo = {.finalTransformation = Eigen::Matrix4f::Identity()};
            skeleton.bones.push_back(newBoneInfo);
        } else {
            //Bone Already Exists
            boneID = skeleton.boneMapping.at(boneName);
        }

        skinning_data.boneOffsetMatrices.try_emplace(boneID, aiMat4ToEigen(mesh->mBones[boneIndex]->mOffsetMatrix));

        aiVertexWeight *weights = mesh->mBones[boneIndex]->mWeights;
        unsigned int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            unsigned int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            for (int i = 0; i < 4; ++i) {
                if (data.boneIDs[vertexId][i] < 0) {
                    data.boneWeights[vertexId][i] = weight;
                    data.boneIDs[vertexId][i] = boneID;
                    break;
                }
            }
        }
    }
}

std::unordered_map<std::string, Animation> ModelLoader::extractAnimations(const aiScene *scene, Model::Skeleton &skeleton) {
    std::unordered_map<std::string, Animation> out;
    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation *anim = scene->mAnimations[i];

        Animation a;
        a.duration = anim->mDuration;
        a.ticksPerSecond = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0f;

        for (unsigned int c = 0; c < anim->mNumChannels; c++) {
            aiNodeAnim *channel = anim->mChannels[c];
            std::string boneName = channel->mNodeName.C_Str();

            BoneAnimation track;

            for (int k = 0; k < channel->mNumPositionKeys; k++) {
                track.positions.push_back({(float) channel->mPositionKeys[k].mTime, aiVec3ToEigen(channel->mPositionKeys[k].mValue)});
            }

            for (int k = 0; k < channel->mNumRotationKeys; k++) {
                track.rotations.push_back({(float) channel->mRotationKeys[k].mTime, aiQuatToEigen(channel->mRotationKeys[k].mValue)});
            }

            for (int k = 0; k < channel->mNumScalingKeys; k++) {
                track.scales.push_back({(float) channel->mScalingKeys[k].mTime, aiVec3ToEigen(channel->mScalingKeys[k].mValue)});
            }

            a.tracks[boneName] = std::move(track);
        }
        std::string anim_name = anim->mName.C_Str();
        
        out.try_emplace(anim_name.substr(anim_name.find_first_of('|') + 1), std::move(a));
    }
    return out;
}

Eigen::Vector4f ModelLoader::colorToVec(aiColor4D *color) {
    Eigen::Vector4f v;
    v.x() = color->r;
    v.y() = color->g;
    v.z() = color->b;
    v.w() = color->a;
    return v;
}

Eigen::Matrix4f ModelLoader::aiMat4ToEigen(const aiMatrix4x4 &m) {
    Eigen::Matrix4f out;

    out(0, 0) = m.a1;
    out(0, 1) = m.a2;
    out(0, 2) = m.a3;
    out(0, 3) = m.a4;
    out(1, 0) = m.b1;
    out(1, 1) = m.b2;
    out(1, 2) = m.b3;
    out(1, 3) = m.b4;
    out(2, 0) = m.c1;
    out(2, 1) = m.c2;
    out(2, 2) = m.c3;
    out(2, 3) = m.c4;
    out(3, 0) = m.d1;
    out(3, 1) = m.d2;
    out(3, 2) = m.d3;
    out(3, 3) = m.d4;

    return out;
}

Eigen::Vector3f ModelLoader::aiVec3ToEigen(const aiVector3D &vec) {
    Eigen::Vector3f v;
    v.x() = vec.x;
    v.y() = vec.y;
    v.z() = vec.z;
    return v;
}

Eigen::Quaternionf ModelLoader::aiQuatToEigen(const aiQuaternion &q) {
    Eigen::Quaternionf quat;
    quat.w() = q.w;
    quat.x() = q.x;
    quat.y() = q.y;
    quat.z() = q.z;
    return quat;
}

constexpr TextureFormat ModelLoader::getTextureFormat(aiTextureType type, int channels) {
    switch (type) {
        case aiTextureType_METALNESS:
        case aiTextureType_AMBIENT_OCCLUSION:
        case aiTextureType_LIGHTMAP:
        case aiTextureType_DIFFUSE_ROUGHNESS:
        case aiTextureType_NORMALS:
            return channels == 3 ? TextureFormat::RGB8 : TextureFormat::RGBA8;

        case aiTextureType_BASE_COLOR:
        case aiTextureType_EMISSIVE:
            return channels == 3 ? TextureFormat::SRGB8 : TextureFormat::SRGBA8;
        default:
            return channels == 3 ? TextureFormat::RGB8 : TextureFormat::RGBA8;
    }
}

}  // namespace ICE
