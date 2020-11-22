//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_MESH_H
#define ICE_MESH_H

#include <Eigen/Dense>
#include <vector>
#include "VertexArray.h"

namespace ICE {
    class Mesh {
    private:
        std::vector<Eigen::Vector3d> vertices, normals;
        std::vector<Eigen::Vector2d> uvCoords;
        std::vector<Eigen::Vector3i> indices;

        VertexArray* vertexArray;
    public:
        Mesh(const std::vector<Eigen::Vector3d> &vertices, const std::vector<Eigen::Vector3d> &normals,
             const std::vector<Eigen::Vector2d> &uvCoords, const std::vector<Eigen::Vector3i> &indices);

        const std::vector<Eigen::Vector3d> &getVertices() const;

        const std::vector<Eigen::Vector3d> &getNormals() const;

        const std::vector<Eigen::Vector2d> &getUVCoords() const;

        const std::vector<Eigen::Vector3i> &getIndices() const;

        const VertexArray* getVertexArray() const;
    };
}


#endif //ICE_MESH_H
