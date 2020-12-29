//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Mesh.h"
#include <Graphics/VertexArray.h>
#include <Util/BufferUtils.h>
#include <iostream>

namespace ICE {
    Mesh::Mesh(const std::vector<Eigen::Vector3f> &vertices, const std::vector<Eigen::Vector3f> &normals,
               const std::vector<Eigen::Vector2f> &uvCoords, const std::vector<Eigen::Vector3i> &indices) : vertices(vertices), normals(normals), uvCoords(uvCoords), indices(indices), boundingBox(vertices) {

        vertexArray = VertexArray::Create();

        auto vertexBuffer = VertexBuffer::Create();
        auto normalsBuffer = VertexBuffer::Create();
        auto uvBuffer = VertexBuffer::Create();
        auto indexBuffer = IndexBuffer::Create();

        vertexBuffer->putData(BufferUtils::CreateFloatBuffer(vertices), 3*vertices.size()*sizeof(float));
        normalsBuffer->putData(BufferUtils::CreateFloatBuffer(normals), 3*normals.size()*sizeof(float));
        uvBuffer->putData(BufferUtils::CreateFloatBuffer(uvCoords), 2*uvCoords.size()*sizeof(float));
        indexBuffer->putData(BufferUtils::CreateIntBuffer(indices), 3*indices.size()*sizeof(int));

        vertexArray->pushVertexBuffer(vertexBuffer, 3);
        vertexArray->pushVertexBuffer(normalsBuffer, 3);
        vertexArray->pushVertexBuffer(uvBuffer, 2);
        vertexArray->setIndexBuffer(indexBuffer);
    }

    const std::vector<Eigen::Vector3f> &Mesh::getVertices() const {
        return vertices;
    }

    const std::vector<Eigen::Vector3f> &Mesh::getNormals() const {
        return normals;
    }

    const std::vector<Eigen::Vector2f> &Mesh::getUVCoords() const {
        return uvCoords;
    }

    const std::vector<Eigen::Vector3i> &Mesh::getIndices() const {
        return indices;
    }

    const VertexArray* Mesh::getVertexArray() const {
        return vertexArray;
    }

    const AABB &Mesh::getBoundingBox() const {
        return boundingBox;
    }
}