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
class Mesh : public Asset {
   public:
    Mesh(const std::vector<Eigen::Vector3f> &vertices, const std::vector<Eigen::Vector3f> &normals, const std::vector<Eigen::Vector2f> &uvCoords,
         const std::vector<Eigen::Vector3i> &indices);
    Mesh(std::vector<Eigen::Vector3f> &&vertices, std::vector<Eigen::Vector3f> &&normals, std::vector<Eigen::Vector2f> &&uvCoords,
         std::vector<Eigen::Vector3i> &&indices);

    const std::vector<Eigen::Vector3f> &getVertices() const;

    const std::vector<Eigen::Vector3f> &getNormals() const;

    const std::vector<Eigen::Vector2f> &getUVCoords() const;

    const std::vector<Eigen::Vector3i> &getIndices() const;

    const std::shared_ptr<VertexArray> getVertexArray() const;
    void setVertexArray(const std::shared_ptr<VertexArray> &vao);

    const AABB &getBoundingBox() const;

    std::string getTypeName() const override;
    AssetType getType() const override;

   private:
    std::vector<Eigen::Vector3f> vertices, normals;
    std::vector<Eigen::Vector2f> uvCoords;
    std::vector<Eigen::Vector3i> indices;
    std::shared_ptr<VertexArray> vertexArray;
    AABB boundingBox;
};
}  // namespace ICE