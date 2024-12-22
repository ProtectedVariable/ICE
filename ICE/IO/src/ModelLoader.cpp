//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ModelLoader.h"

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

        materials.push_back(extractMaterial(material));

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

AssetUID extractMaterial(const aiMaterial *material) {
    auto mtl = std::make_shared<Material>();
    mtl->setShader()
}
}  // namespace ICE
