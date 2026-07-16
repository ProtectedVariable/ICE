//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ModelLoader.h"

#include <AssetBank.h>
#include <Logger.h>
#include <Material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstdlib>
#include <cstring>

#include <assimp/Importer.hpp>
#include <cassert>
#include <iostream>

namespace ICE {
std::shared_ptr<Model> ModelLoader::load(const std::vector<std::filesystem::path> &file) {
    // Resolve the one bank value the parse needs on the calling (main) thread, then stage (pure) and
    // commit. Splitting these is what lets the async import pipeline run stage() on a worker while
    // keeping bank mutation on the main thread; the synchronous path here is behavior-identical to
    // the previous single-pass loader.
    AssetUID pbr_shader_uid = ref_bank.getUID(AssetPath::WithTypePrefix<Shader>("pbr"));
    StagedModel staged = stage(file, pbr_shader_uid);
    return commit(staged, ref_bank);
}

StagedModel ModelLoader::stage(const std::vector<std::filesystem::path> &file, AssetUID pbr_shader_uid) {
    StagedModel staged;
    if (file.empty()) {
        return staged;  // valid == false
    }
    Assimp::Importer importer;

    const aiScene *scene =
        importer.ReadFile(file[0].string(),
                          aiProcess_OptimizeGraph | aiProcess_FlipUVs | aiProcess_ValidateDataStructure | aiProcess_SortByPType
                              | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_LimitBoneWeights);

    if (scene == nullptr || scene->mRootNode == nullptr) {
        Logger::Log(Logger::ERROR, "IO", "Could not load model '%s': %s", file[0].string().c_str(), importer.GetErrorString());
        return staged;  // valid == false -> commit() does nothing
    }

    staged.sources = file;
    staged.skeleton.globalInverseTransform = aiMat4ToEigen(scene->mRootNode->mTransformation).inverse();
    for (int m = 0; m < scene->mNumMeshes; m++) {
        auto mesh = scene->mMeshes[m];
        auto material = scene->mMaterials[mesh->mMaterialIndex];
        auto model_name = file[0].filename().stem().string();
        staged.meshes.push_back(stageMesh(mesh, model_name, scene, staged.skeleton));
        staged.materials.push_back(stageMaterial(material, model_name, scene, pbr_shader_uid));
    }
    std::unordered_set<std::string> used_node_names;
    processNode(scene->mRootNode, staged.nodes, staged.skeleton, used_node_names, Eigen::Matrix4f::Identity());

    if (scene->HasAnimations()) {
        staged.animations = extractAnimations(scene, staged.skeleton);
        staged.hasAnimations = true;
    }
    staged.valid = true;
    return staged;
}

std::shared_ptr<Model> ModelLoader::commit(StagedModel &staged, AssetBank &bank) {
    if (!staged.valid) {
        return nullptr;
    }
    std::vector<AssetUID> meshes;
    std::vector<AssetUID> materials;
    meshes.reserve(staged.meshes.size());
    materials.reserve(staged.materials.size());
    // Commit mesh[i] then material[i] in scene order so UID assignment (a single bank-wide counter)
    // is identical to the original single-pass loader -- required for stable re-import and project
    // files.
    for (size_t i = 0; i < staged.meshes.size(); i++) {
        meshes.push_back(commitMesh(staged.meshes[i], bank));
        materials.push_back(commitMaterial(staged.materials[i], bank));
    }
    auto model = std::make_shared<Model>(staged.nodes, meshes, materials);

    if (staged.hasAnimations) {
        model->setAnimations(staged.animations);
        model->setSkeleton(staged.skeleton);
    }
    model->setSources(staged.sources);
    return model;
}

int ModelLoader::processNode(const aiNode *ainode, std::vector<Model::Node> &nodes, Model::Skeleton &skeleton,
                             std::unordered_set<std::string> &used_names, const Eigen::Matrix4f &parent_transform) {
    std::string name = ainode->mName.C_Str();
    if (used_names.contains(name)) {
        // Suffix with an incrementing counter checked against existing names. The old
        // "_<set size>" suffix could still collide with a node that already had that name.
        const std::string base = name;
        int counter = 1;
        do {
            name = base + "_" + std::to_string(counter++);
        } while (used_names.contains(name));
    }
    used_names.insert(name);

    Model::Node node;
    node.name = name;

    aiMatrix4x4 local = ainode->mTransformation;
    node.localTransform = aiMat4ToEigen(local);

    for (unsigned int i = 0; i < ainode->mNumMeshes; ++i) {
        unsigned int mesh_idx = ainode->mMeshes[i];
        node.meshIndices.push_back(mesh_idx);
    }
    auto insert_pos = nodes.size();

    nodes.push_back(node);

    for (unsigned int c = 0; c < ainode->mNumChildren; ++c) {
        const aiNode *child = ainode->mChildren[c];
        int child_pos = processNode(child, nodes, skeleton, used_names, parent_transform * node.localTransform);
        nodes.at(insert_pos).children.push_back(child_pos);
    }
    return insert_pos;
}

StagedMesh ModelLoader::stageMesh(const aiMesh *mesh, const std::string &model_name, const aiScene *scene, Model::Skeleton &skeleton) {
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

    std::unordered_map<int, Eigen::Matrix4f> inverseBindMatrices;
    if (mesh->HasBones()) {
        inverseBindMatrices = extractBoneData(mesh, data, skeleton);
    }
    auto mesh_ = std::make_shared<Mesh>(std::move(data));
    for (const auto &[boneID, ibm] : inverseBindMatrices) {
        mesh_->setIBM(boneID, ibm);
    }

    AssetPath mesh_path = AssetPath::WithTypePrefix<Mesh>(model_name + "/" + mesh->mName.C_Str());
    return StagedMesh{mesh_path, mesh_};
}

AssetUID ModelLoader::commitMesh(StagedMesh &staged, AssetBank &bank) {
    AssetUID mesh_id = 0;
    if (mesh_id = bank.getUID(staged.path); mesh_id != 0) {
        // Re-import: drop the old asset (fires the eviction listener so the GPU upload is released)
        // and re-add under the same UID so existing references (scenes, components) stay valid.
        bank.removeAsset(staged.path);
        bank.addAssetWithSpecificUID(staged.path, staged.mesh, mesh_id);
    } else {
        bank.addAsset(staged.path, staged.mesh);
        mesh_id = bank.getUID(staged.path);
    }
    return mesh_id;
}

StagedMaterial ModelLoader::stageMaterial(const aiMaterial *material, const std::string &model_name, const aiScene *scene, AssetUID pbr_shader_uid) {
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
    mtl->setShader(pbr_shader_uid);
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

    // AssetPath has no default constructor, so build the aggregate with the path in place (rather
    // than default-construct then assign).
    StagedMaterial staged{AssetPath::WithTypePrefix<Material>(bank_name), mtl, {}};

    // Stage each present texture with the uniform slots it feeds. Order matches the original loader
    // so texture UID assignment at commit is unchanged. The hasXMap/xMap uniforms are set in
    // commitMaterial once the textures have UIDs.
    stageTexture(staged, material, bank_name + "/ao_map", scene, aiTextureType_LIGHTMAP, "material.hasAoMap", "material.aoMap");
    stageTexture(staged, material, bank_name + "/diffuse_map", scene, aiTextureType_BASE_COLOR, "material.hasBaseColorMap", "material.baseColorMap");
    stageTexture(staged, material, bank_name + "/metallic_map", scene, aiTextureType_METALNESS, "material.hasMetallicMap", "material.metallicMap");
    stageTexture(staged, material, bank_name + "/roughness_map", scene, aiTextureType_DIFFUSE_ROUGHNESS, "material.hasRoughnessMap",
                 "material.roughnessMap");
    stageTexture(staged, material, bank_name + "/normal_map", scene, aiTextureType_NORMALS, "material.hasNormalMap", "material.normalMap");
    stageTexture(staged, material, bank_name + "/emissive_map", scene, aiTextureType_EMISSIVE, "material.hasEmissiveMap", "material.emissiveMap");

    return staged;
}

AssetUID ModelLoader::commitMaterial(StagedMaterial &staged, AssetBank &bank) {
    // Commit the material's textures first (this also replaces them, preserving UIDs, on re-import)
    // and wire the resulting UIDs into the material uniforms.
    for (auto &tex : staged.textures) {
        AssetUID tex_id = commitTexture(tex, bank);
        staged.material->setUniform(tex.has_uniform, 1);
        staged.material->setUniform(tex.map_uniform, tex_id);
    }

    if (bank.getUID(staged.path) != 0) {
        // Material already present (e.g. shared by several meshes): dedupe by path, as before.
        return bank.getUID(staged.path);
    }

    bank.addAsset(staged.path, staged.material);
    return bank.getUID(staged.path);
}

void ModelLoader::stageTexture(StagedMaterial &staged_material, const aiMaterial *material, const std::string &tex_path, const aiScene *scene,
                               aiTextureType type, const std::string &has_uniform, const std::string &map_uniform) {
    aiString texture_file;
    if (material->Get(AI_MATKEY_TEXTURE(type, 0), texture_file) == aiReturn_SUCCESS) {
        if (auto texture = scene->GetEmbeddedTexture(texture_file.C_Str())) {
            unsigned char *data = reinterpret_cast<unsigned char *>(texture->pcData);
            void *data2 = nullptr;
            int width = texture->mWidth;
            int height = texture->mHeight;
            int channels = 4;
            if (height == 0) {
                // Compressed in memory: decode with stbi into an owned RGBA buffer.
                data2 = stbi_load_from_memory(data, texture->mWidth, &width, &height, &channels, 4);
                channels = 4;
            } else {
                // Uncompressed aiTexel (BGRA/RGBA8888) data lives in the aiScene, which is
                // freed when this Importer is destroyed -- and the GPU upload happens later.
                // Copy it into an owned buffer so the Texture2D remains valid.
                size_t byte_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
                data2 = malloc(byte_size);
                if (data2 != nullptr) {
                    memcpy(data2, data, byte_size);
                }
            }
            // take_ownership=true: both branches produce a free-compatible (stbi/malloc) buffer.
            auto texture_ice = std::make_shared<Texture2D>(data2, width, height, getTextureFormat(type, channels), true);
            staged_material.textures.push_back(StagedTexture{AssetPath::WithTypePrefix<Texture2D>(tex_path), texture_ice, has_uniform, map_uniform});
        } else {
            //regular file, check if it exists and read it
            //TODO :)
        }
    }
}

AssetUID ModelLoader::commitTexture(StagedTexture &staged, AssetBank &bank) {
    AssetUID tex_id = 0;
    if (tex_id = bank.getUID(staged.path); tex_id != 0) {
        bank.removeAsset(staged.path);
        bank.addAssetWithSpecificUID(staged.path, staged.texture, tex_id);
    } else {
        bank.addAsset(staged.path, staged.texture);
        tex_id = bank.getUID(staged.path);
    }
    return tex_id;
}

std::unordered_map<int, Eigen::Matrix4f> ModelLoader::extractBoneData(const aiMesh *mesh, MeshData &data, Model::Skeleton &skeleton) {
    std::unordered_map<int, Eigen::Matrix4f> inverseBindMatrices;
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        int boneID = -1;
        // If the bone is new (hasn't been added by a previous mesh)
        if (!skeleton.boneMapping.contains(boneName)) {
            boneID = skeleton.boneMapping.size();
            skeleton.boneMapping[boneName] = boneID;
        } else {
            //Bone Already Exists
            boneID = skeleton.boneMapping.at(boneName);
        }

        inverseBindMatrices.try_emplace(boneID, aiMat4ToEigen(mesh->mBones[boneIndex]->mOffsetMatrix));

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
    return inverseBindMatrices;
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

TextureFormat ModelLoader::getTextureFormat(aiTextureType type, int channels) {
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
