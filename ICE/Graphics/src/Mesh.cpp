//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Mesh.h"

#include <BufferUtils.h>
#include <VertexArray.h>

#include <iostream>

namespace ICE {
Mesh::Mesh(const MeshData &data) : m_data(data), boundingBox(data.vertices) {
    for (const auto &boneIDs : m_data.boneIDs) {
        for (int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
            if (boneIDs[i] != INVALID_BONE_ID) {
                m_has_bones = true;
                break;
            }
        }
        if (m_has_bones)
            break;
    }
}
Mesh::Mesh(MeshData &&data) : m_data(std::move(data)), boundingBox(getVertices()) {
    for (const auto & boneIDs : m_data.boneIDs) {
        for (int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
            if (boneIDs[i] != INVALID_BONE_ID) {
                m_has_bones = true;
                break;
            }
        }
        if (m_has_bones)
            break;
    }
}
const std::vector<Eigen::Vector3f> &Mesh::getVertices() const {
    return m_data.vertices;
}

const std::vector<Eigen::Vector3f> &Mesh::getNormals() const {
    return m_data.normals;
}

const std::vector<Eigen::Vector2f> &Mesh::getUVCoords() const {
    return m_data.uvCoords;
}

const std::vector<Eigen::Vector3i> &Mesh::getIndices() const {
    return m_data.indices;
}

const std::shared_ptr<VertexArray> Mesh::getVertexArray() const {
    return vertexArray;
}

void Mesh::setVertexArray(const std::shared_ptr<VertexArray> &vao) {
    this->vertexArray = vao;
}

const AABB &Mesh::getBoundingBox() const {
    return boundingBox;
}

std::string Mesh::getTypeName() const {
    return "Mesh";
}
AssetType Mesh::getType() const {
    return AssetType::EMesh;
}
}  // namespace ICE