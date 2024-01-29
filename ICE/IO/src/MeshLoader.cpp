//
// Created by Thomas Ibanez on 31.07.21.
//

#include "MeshLoader.h"

#include <OBJLoader.h>
#include <BufferUtils.h>

namespace ICE {
std::shared_ptr<Mesh> MeshLoader::load(const std::vector<std::filesystem::path> &file) {
    auto mesh = OBJLoader::loadFromOBJ(file[0].string());
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

    return mesh;
}
}  // namespace ICE
