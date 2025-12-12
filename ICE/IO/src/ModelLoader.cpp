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

namespace ICE {
std::shared_ptr<Model> ModelLoader::load(const std::vector<std::filesystem::path> &file) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(file[0].string(),
                                             aiProcess_FlipUVs | aiProcess_ValidateDataStructure | aiProcess_SortByPType | aiProcess_GenSmoothNormals
                                                 | aiProcess_CalcTangentSpace | aiProcess_Triangulate);

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<AssetUID> materials;
    std::vector<Model::Node> nodes;

    for (int m = 0; m < scene->mNumMeshes; m++) {
        auto mesh = scene->mMeshes[m];
        auto material = scene->mMaterials[mesh->mMaterialIndex];
        auto model_name = file[0].filename().stem().string();
        meshes.push_back(extractMesh(mesh, model_name, scene));
        materials.push_back(extractMaterial(material, model_name, scene));
        meshes.back()->setSources(file);
    }
    processNode(scene->mRootNode, nodes);
    auto model = std::make_shared<Model>(nodes, meshes, materials);
    model->setSources(file);
    return model;
}

int ModelLoader::processNode(const aiNode *ainode, std::vector<Model::Node> &nodes) {
    Model::Node node;
    node.name = ainode->mName.C_Str();
    // compute local transform (relative to parent)
    aiMatrix4x4 local = ainode->mTransformation;
    node.localTransform = aiMat4ToEigen(local);  // store local transform (or AiMat4ToGlm(world) if you want world)

    for (unsigned int i = 0; i < ainode->mNumMeshes; ++i) {
        unsigned int mesh_idx = ainode->mMeshes[i];
        node.meshIndices.push_back(mesh_idx);
    }
    auto insert_pos = nodes.size();
    nodes.push_back(node);

    for (unsigned int c = 0; c < ainode->mNumChildren; ++c) {
        const aiNode *child = ainode->mChildren[c];
        int child_pos = processNode(child, nodes);
        nodes.at(insert_pos).children.push_back(child_pos);
    }
    return insert_pos;
}

std::shared_ptr<Mesh> ModelLoader::extractMesh(const aiMesh *mesh, const std::string &model_name, const aiScene *scene) {
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

    if (mesh->HasBones()) {
        extractBoneWeightForVertices(mesh, scene, data);
    }

    auto mesh_ = std::make_shared<Mesh>(data);

    auto vertexArray = graphics_factory->createVertexArray();

    auto vertexBuffer = graphics_factory->createVertexBuffer();
    auto normalsBuffer = graphics_factory->createVertexBuffer();
    auto uvBuffer = graphics_factory->createVertexBuffer();
    auto tangentBuffer = graphics_factory->createVertexBuffer();
    auto biTangentBuffer = graphics_factory->createVertexBuffer();
    auto boneIDBuffer = graphics_factory->createVertexBuffer();
    auto boneWeightBuffer = graphics_factory->createVertexBuffer();
    auto indexBuffer = graphics_factory->createIndexBuffer();

    vertexBuffer->putData(BufferUtils::CreateFloatBuffer(data.vertices), 3 * data.vertices.size() * sizeof(float));
    normalsBuffer->putData(BufferUtils::CreateFloatBuffer(data.normals), 3 * data.normals.size() * sizeof(float));
    tangentBuffer->putData(BufferUtils::CreateFloatBuffer(data.tangents), 3 * data.tangents.size() * sizeof(float));
    biTangentBuffer->putData(BufferUtils::CreateFloatBuffer(data.bitangents), 3 * data.bitangents.size() * sizeof(float));
    boneIDBuffer->putData(BufferUtils::CreateIntBuffer(data.boneIDs), 4 * data.boneIDs.size() * sizeof(int));
    boneWeightBuffer->putData(BufferUtils::CreateFloatBuffer(data.boneWeights), 4 * data.boneWeights.size() * sizeof(float));
    uvBuffer->putData(BufferUtils::CreateFloatBuffer(data.uvCoords), 2 * data.uvCoords.size() * sizeof(float));
    indexBuffer->putData(BufferUtils::CreateIntBuffer(data.indices), 3 * data.indices.size() * sizeof(int));

    vertexArray->pushVertexBuffer(vertexBuffer, 0, 3);
    vertexArray->pushVertexBuffer(normalsBuffer, 1, 3);
    vertexArray->pushVertexBuffer(uvBuffer, 2, 2);
    vertexArray->pushVertexBuffer(tangentBuffer, 3, 3);
    vertexArray->pushVertexBuffer(biTangentBuffer, 4, 3);
    vertexArray->pushVertexBuffer(boneIDBuffer, 5, 4);
    vertexArray->pushVertexBuffer(boneWeightBuffer, 6, 4);
    vertexArray->setIndexBuffer(indexBuffer);

    mesh_->setVertexArray(vertexArray);
    return mesh_;
}

AssetUID ModelLoader::extractMaterial(const aiMaterial *material, const std::string &model_name, const aiScene *scene) {
    auto mtl_name = material->GetName();
    if (mtl_name.length == 0) {
        mtl_name = "DefaultMat";
    }
    auto bank_name = model_name + "/" + mtl_name.C_Str();
    auto mtl = std::make_shared<Material>();
    mtl->setUniform("material.use_diffuse_map", false);
    mtl->setUniform("material.use_ambient_map", false);
    mtl->setUniform("material.use_specular_map", false);
    mtl->setUniform("material.use_normal_map", false);
    if (auto ambient_map = extractTexture(material, bank_name + "/ambient_map", scene, aiTextureType_AMBIENT); ambient_map != 0) {
        mtl->setUniform("material.ambient_map", ambient_map);
        mtl->setUniform("material.use_ambient_map", true);
    }
    if (auto diffuse_tex = extractTexture(material, bank_name + "/diffuse_map", scene, aiTextureType_DIFFUSE); diffuse_tex != 0) {
        mtl->setUniform("material.diffuse_map", diffuse_tex);
        mtl->setUniform("material.use_diffuse_map", true);
    }
    if (auto specular_tex = extractTexture(material, bank_name + "/specular_map", scene, aiTextureType_SPECULAR); specular_tex != 0) {
        mtl->setUniform("material.specular_map", specular_tex);
        mtl->setUniform("material.use_specular_map", true);
    }
    if (auto normal_tex = extractTexture(material, bank_name + "/normal_map", scene, aiTextureType_NORMALS); normal_tex != 0) {
        mtl->setUniform("material.normal_map", normal_tex);
        mtl->setUniform("material.use_normal_map", true);
    }

    if (ref_bank.getUID(AssetPath::WithTypePrefix<Material>(bank_name)) != 0) {
        return ref_bank.getUID(AssetPath::WithTypePrefix<Material>(bank_name));
    }
    mtl->setShader(ref_bank.getUID(AssetPath::WithTypePrefix<Shader>("phong")));

    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D ambient;
    ai_real alpha = 1.0;

    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS)
        mtl->setUniform("material.albedo", colorToVec(&diffuse));
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS)
        mtl->setUniform("material.specular", colorToVec(&specular));
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == aiReturn_SUCCESS)
        mtl->setUniform("material.ambient", colorToVec(&ambient));
    if (aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &alpha) == aiReturn_SUCCESS)
        mtl->setUniform("material.alpha", std::max(alpha, 1.0f));

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
            int channels = 0;
            if (height == 0) {
                //Compressed memory, use stbi to load
                data2 = stbi_load_from_memory(data, texture->mWidth, &width, &height, &channels, 4);
            } else {
                data2 = data;
            }
            auto texture_ice = m_graphics_factory->createTexture2D(data2, width, height, TextureFormat::RGBA);
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

void extractBoneWeightForVertices(const aiMesh *mesh, const aiScene *scene, MeshData &data) {
    unsigned int numBones = mesh->mNumBones > MAX_BONES ? MAX_BONES : mesh->mNumBones;

    for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex) {
        int boneID = boneIndex;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

        // Get all vertex weights for current bone
        aiVertexWeight *weights = mesh->mBones[boneIndex]->mWeights;
        unsigned int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        // For each weight at vertex x for current bone
        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            unsigned int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            // Update four most influential bones
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

}  // namespace ICE
