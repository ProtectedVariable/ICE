//
// Created by Thomas Ibanez on 16.11.20.
//
#define TINYOBJLOADER_IMPLEMENTATION

#include <iostream>
#include "OBJLoader.h"
#include "Logger.h"
#include "ICEException.h"

namespace ICE {
    Mesh* OBJLoader::loadFromOBJ(const std::string &path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

        if (!warn.empty()) {
            Logger::Log(Logger::WARNING, "Util", warn.c_str());
        }

        if (!err.empty()) {
            Logger::Log(Logger::ERROR, "Util", err.c_str());
        }
        if(!ret) {
            Logger::Log(Logger::ERROR, "Util", "Couldn't load model");
            throw ICEException();
        }
        auto vertices = std::vector<Eigen::Vector3f>();
        auto normals = std::vector<Eigen::Vector3f>();
        auto uvs = std::vector<Eigen::Vector2f>();
        auto indices = std::vector<Eigen::Vector3i>();
        int s = 0;
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            assert(fv == 3); //We only want triangle faces for now
            Eigen::Vector3f trianglesVertices[3] = {
                    Eigen::Vector3f(),
                    Eigen::Vector3f(),
                    Eigen::Vector3f()
            };
            bool computedNormals = false;
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
                tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
                tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];
                tinyobj::real_t nx,ny,nz;
                if(!attrib.normals.empty()) {
                    nx = attrib.normals[3*idx.normal_index+0];
                    ny = attrib.normals[3*idx.normal_index+1];
                    nz = attrib.normals[3*idx.normal_index+2];
                } else {
                    computedNormals = true;
                    trianglesVertices[v].x() = vx;
                    trianglesVertices[v].y() = vy;
                    trianglesVertices[v].z() = vz;
                }
                tinyobj::real_t tx,ty;
                if(!attrib.texcoords.empty()) {
                    tx = attrib.texcoords[2*idx.texcoord_index+0];
                    ty = attrib.texcoords[2*idx.texcoord_index+1];
                }
                vertices.emplace_back(vx,vy,vz);
                if(!computedNormals) {
                    auto normal = Eigen::Vector3f(nx,ny,nz).normalized();
                    normals.emplace_back(normal);
                }
                uvs.emplace_back(tx,ty);
            }
            if(computedNormals) {
                for (size_t v = 0; v < fv; v++) {
                    auto n = (trianglesVertices[1] - trianglesVertices[0]).cross(trianglesVertices[2]-trianglesVertices[0]);
                    n.normalize();
                    normals.push_back(n);
                    normals.push_back(n);
                    normals.push_back(n);
                }
            }
            index_offset += fv;
        }
        for (long i = 0; i < shapes[s].mesh.indices.size(); i+=3) {
            indices.emplace_back(i, i + 1, i +2);
        }
        /*
        tinyobj::mesh_t mesh = shapes[s].mesh;
        vertices.reserve(attrib.vertices.size());
        normals.reserve(attrib.vertices.size());
        uvs.reserve(attrib.vertices.size());
        for (long i = 0; i < attrib.vertices.size(); i+=3) {
            vertices.emplace_back(attrib.vertices[i],attrib.vertices[i+1],attrib.vertices[i+2]);
        }
        for(long i = 0; i <  mesh.indices.size(); i+=3) {
            normals.emplace_back(0,0,0);
        }

        for (long i = 0; i < mesh.indices.size(); i+=3) {
            indices.emplace_back(mesh.indices[i].vertex_index, mesh.indices[i + 1].vertex_index, mesh.indices[i + 2].vertex_index);
            for(int j = 0; j < 3; j++) {
                normals[mesh.indices[i].vertex_index] = Eigen::Vector3f(attrib.normals[3*mesh.indices[i+j].normal_index], attrib.normals[3*mesh.indices[i + j].normal_index+1], attrib.normals[3*mesh.indices[i + j].normal_index+2]);
            }
        }
        */
        return new Mesh(vertices, normals, uvs, indices);
    }
}