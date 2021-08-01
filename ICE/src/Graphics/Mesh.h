//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_MESH_H
#define ICE_MESH_H

#include <Eigen/Dense>
#include <vector>
#include <Math/AABB.h>
#include <Assets/Asset.h>
#include "VertexArray.h"

namespace ICE {
    class Mesh : public Asset {
    public:
        Mesh(const std::vector<Eigen::Vector3f> &vertices, const std::vector<Eigen::Vector3f> &normals,
             const std::vector<Eigen::Vector2f> &uvCoords, const std::vector<Eigen::Vector3i> &indices);

        const std::vector<Eigen::Vector3f> &getVertices() const;

        const std::vector<Eigen::Vector3f> &getNormals() const;

        const std::vector<Eigen::Vector2f> &getUVCoords() const;

        const std::vector<Eigen::Vector3i> &getIndices() const;

        const VertexArray* getVertexArray() const;

        const AABB &getBoundingBox() const;

        std::string getTypeName() override;

    private:
        std::vector<Eigen::Vector3f> vertices, normals;
        std::vector<Eigen::Vector2f> uvCoords;
        std::vector<Eigen::Vector3i> indices;
        VertexArray* vertexArray;
        AABB boundingBox;
    };
}


#endif //ICE_MESH_H
