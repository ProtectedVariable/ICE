//
// Created by Thomas Ibanez on 31.07.21.
//

#include "ModelLoader.h"

#include <BufferUtils.h>
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
        auto mesh0 = scene->mMeshes[m];
        for (int i = 0; i < mesh0->mNumVertices; i++) {
            auto v = mesh0->mVertices[i];
            auto n = mesh0->mNormals[i];
            Eigen::Vector2f uv(0, 0);
            if (mesh0->mTextureCoords[0] != nullptr) {
                auto uv_file = mesh0->mTextureCoords[0][i];
                uv.x() = uv_file.x;
                uv.y() = uv_file.y;
            }
            vertices.emplace_back(v.x, v.y, v.z);
            normals.emplace_back(n.x, n.y, n.z);
            uvs.push_back(uv);
        }
        for (int i = 0; i < mesh0->mNumFaces; i++) {
            auto f = mesh0->mFaces[i];
            assert(f.mNumIndices == 3);
            indices.emplace_back(f.mIndices[0], f.mIndices[1], f.mIndices[2]);
        }

        auto mesh = std::make_shared<Mesh>(vertices, normals, uvs, indices);

        mesh->setSources(file);

        auto vertexArray = graphics_factory->createVertexArray();

        auto vertexBuffer = graphics_factory->createVertexBuffer();
        auto normalsBuffer = graphics_factory->createVertexBuffer();
        auto uvBuffer = graphics_factory->createVertexBuffer();
        auto indexBuffer = graphics_factory->createIndexBuffer();

        vertexBuffer->putData(BufferUtils::CreateFloatBuffer(mesh->getVertices()), 3 * mesh->getVertices().size() * sizeof(float));
        normalsBuffer->putData(BufferUtils::CreateFloatBuffer(mesh->getNormals()), 3 * mesh->getNormals().size() * sizeof(float));
        uvBuffer->putData(BufferUtils::CreateFloatBuffer(mesh->getUVCoords()), 2 * mesh->getUVCoords().size() * sizeof(float));
        indexBuffer->putData(BufferUtils::CreateIntBuffer(mesh->getIndices()), 3 * mesh->getIndices().size() * sizeof(int));

        vertexArray->pushVertexBuffer(vertexBuffer, 3);
        vertexArray->pushVertexBuffer(normalsBuffer, 3);
        vertexArray->pushVertexBuffer(uvBuffer, 2);
        vertexArray->setIndexBuffer(indexBuffer);

        mesh->setVertexArray(vertexArray);
    }
    return mesh;
}
}  // namespace ICE
