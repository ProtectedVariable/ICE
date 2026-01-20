//
// Created by Thomas Ibanez on 16.11.20.
//

#pragma once

#include <AABB.h>
#include <Asset.h>

#include <Eigen/Dense>
#include <vector>

#include "VertexArray.h"

namespace ICE {

constexpr int MAX_BONES_PER_VERTEX = 4;
constexpr int MAX_BONES = 100;
constexpr int INVALID_BONE_ID = -1;

struct MeshData {
    std::vector<Eigen::Vector3f> vertices;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Eigen::Vector2f> uvCoords;
    std::vector<Eigen::Vector3f> tangents;
    std::vector<Eigen::Vector3f> bitangents;
    std::vector<Eigen::Vector4i> boneIDs;
    std::vector<Eigen::Vector4f> boneWeights;
    std::vector<Eigen::Vector3i> indices;
};

class Mesh : public Asset {
   public:
    Mesh(const MeshData &data);
    Mesh(MeshData &&data);

    const MeshData &getMeshData() const { return m_data; }

    const std::vector<Eigen::Vector3f> &getVertices() const;

    const std::vector<Eigen::Vector3f> &getNormals() const;

    const std::vector<Eigen::Vector2f> &getUVCoords() const;

    const std::vector<Eigen::Vector3i> &getIndices() const;

    const AABB &getBoundingBox() const;

    std::string getTypeName() const override;
    AssetType getType() const override;

   private:
    MeshData m_data;
    AABB boundingBox;
};
}  // namespace ICE