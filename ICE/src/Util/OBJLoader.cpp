//
// Created by Thomas Ibanez on 16.11.20.
//

#include "OBJLoader.h"

namespace ICE {
    Mesh *OBJLoader::loadFromOBJ(std::string path) {
        bool loadOK = internalLoader.LoadFile(path);
        if(loadOK) {
            auto vertices = std::vector<Eigen::Vector3d>();
            auto normals = std::vector<Eigen::Vector3d>();
            auto texCoords = std::vector<Eigen::Vector2d>();
            auto indices = std::vector<Eigen::Vector3i>();
            objl::Mesh m = internalLoader.LoadedMeshes[0];
            for (auto &Vertice : m.Vertices) {
                vertices.push_back(Eigen::Vector3d(Vertice.Position.X, Vertice.Position.Y, Vertice.Position.Z));
                normals.push_back(Eigen::Vector3d(Vertice.Normal.X, Vertice.Normal.Y, Vertice.Normal.Z));
                texCoords.push_back(Eigen::Vector2d(Vertice.TextureCoordinate.X, Vertice.TextureCoordinate.Y));
            }


            for (int j = 0; j < m.Indices.size(); j += 3) {
                indices.push_back(Eigen::Vector3i(m.Indices[j],m.Indices[j+1],m.Indices[j+2]));
            }

            return new Mesh(vertices, normals, texCoords, indices);
        }
        return nullptr;
    }

    ICE::OBJLoader::OBJLoader() : internalLoader(objl::Loader()) {}
}