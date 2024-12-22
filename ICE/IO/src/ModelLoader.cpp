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

    const aiScene *scene = importer.ReadFile(file[0].string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<AssetUID> materials;

    for (int m = 0; m < scene->mNumMeshes; m++) {
        auto vertices = std::vector<Eigen::Vector3f>();
        auto normals = std::vector<Eigen::Vector3f>();
        auto uvs = std::vector<Eigen::Vector2f>();
        auto indices = std::vector<Eigen::Vector3i>();
        // Loop over faces(polygon)
        auto mesh = scene->mMeshes[m];
        auto material = scene->mMaterials[mesh->mMaterialIndex];

        materials.push_back(extractMaterial(material, file[0].filename().stem().string()));

        for (int i = 0; i < mesh->mNumVertices; i++) {
            auto v = mesh->mVertices[i];
            auto n = mesh->mNormals[i];
            Eigen::Vector2f uv(0, 0);
            if (mesh->mTextureCoords[0] != nullptr) {
                auto uv_file = mesh->mTextureCoords[0][i];
                uv.x() = uv_file.x;
                uv.y() = uv_file.y;
            }
            vertices.emplace_back(v.x, v.y, v.z);
            normals.emplace_back(n.x, n.y, n.z);
            uvs.push_back(uv);
        }
        for (int i = 0; i < mesh->mNumFaces; i++) {
            auto f = mesh->mFaces[i];
            assert(f.mNumIndices == 3);
            indices.emplace_back(f.mIndices[0], f.mIndices[1], f.mIndices[2]);
        }

        meshes.push_back(std::make_shared<Mesh>(vertices, normals, uvs, indices));

        meshes.back()->setSources(file);

        auto vertexArray = graphics_factory->createVertexArray();

        auto vertexBuffer = graphics_factory->createVertexBuffer();
        auto normalsBuffer = graphics_factory->createVertexBuffer();
        auto uvBuffer = graphics_factory->createVertexBuffer();
        auto indexBuffer = graphics_factory->createIndexBuffer();

        vertexBuffer->putData(BufferUtils::CreateFloatBuffer(meshes.back()->getVertices()), 3 * meshes.back()->getVertices().size() * sizeof(float));
        normalsBuffer->putData(BufferUtils::CreateFloatBuffer(meshes.back()->getNormals()), 3 * meshes.back()->getNormals().size() * sizeof(float));
        uvBuffer->putData(BufferUtils::CreateFloatBuffer(meshes.back()->getUVCoords()), 2 * meshes.back()->getUVCoords().size() * sizeof(float));
        indexBuffer->putData(BufferUtils::CreateIntBuffer(meshes.back()->getIndices()), 3 * meshes.back()->getIndices().size() * sizeof(int));

        vertexArray->pushVertexBuffer(vertexBuffer, 3);
        vertexArray->pushVertexBuffer(normalsBuffer, 3);
        vertexArray->pushVertexBuffer(uvBuffer, 2);
        vertexArray->setIndexBuffer(indexBuffer);

        meshes.back()->setVertexArray(vertexArray);
    }
    return std::make_shared<Model>(meshes, materials);
}

AssetUID ModelLoader::extractMaterial(const aiMaterial *material, const std::string &model_name) {
    auto mtl = std::make_shared<Material>();
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
        mtl->setUniform("material.alpha", alpha);
    mtl->setUniform("material.use_diffuse_map", false);
    mtl->setUniform("material.use_ambient_map", false);
    mtl->setUniform("material.use_specular_map", false);
    mtl->setUniform("material.use_normal_map", false);

    auto bank_name = model_name + "-" + material->GetName().C_Str();
    ref_bank.addAsset<Material>(bank_name, mtl);
    return ref_bank.getUID(AssetPath::WithTypePrefix<Material>(bank_name));
}

Eigen::Vector4f ModelLoader::colorToVec(aiColor4D *color) {
    Eigen::Vector4f v;
    v.x() = color->r;
    v.y() = color->g;
    v.z() = color->b;
    v.w() = color->a;
    return v;
}

}  // namespace ICE
