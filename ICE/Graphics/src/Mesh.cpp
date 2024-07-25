//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Mesh.h"

#include <BufferUtils.h>
#include <VertexArray.h>

#include <iostream>

namespace ICE {
Mesh::Mesh(const std::vector<Eigen::Vector3f> &vertices, const std::vector<Eigen::Vector3f> &normals, const std::vector<Eigen::Vector2f> &uvCoords,
           const std::vector<Eigen::Vector3i> &indices)
    : vertices(vertices),
      normals(normals),
      uvCoords(uvCoords),
      indices(indices),
      boundingBox(vertices) {
}
Mesh::Mesh(std::vector<Eigen::Vector3f> &&vertices, std::vector<Eigen::Vector3f> &&normals, std::vector<Eigen::Vector2f> &&uvCoords,
           std::vector<Eigen::Vector3i> &&indices)
    : vertices(std::move(vertices)),
      normals(std::move(normals)),
      uvCoords(std::move(uvCoords)),
      indices(std::move(indices)),
      boundingBox(this->vertices) {
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
void Mesh::load() {
}
void Mesh::unload() {
}
}  // namespace ICE