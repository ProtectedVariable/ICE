//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Mesh.h"

namespace ICE {
    Mesh::Mesh(const std::vector<Eigen::Vector3d> &vertices, const std::vector<Eigen::Vector3d> &normals,
               const std::vector<Eigen::Vector2d> &uvCoords, const std::vector<Eigen::Vector3i> &indices) : vertices(vertices), normals(normals), uvCoords(uvCoords), indices(indices) {}
}