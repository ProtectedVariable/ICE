#include "GPURegistry.h"

#include <BufferUtils.h>

namespace ICE {
GPURegistry::GPURegistry(const std::shared_ptr<GraphicsFactory> &factory) : m_graphics_factory(factory) {
}

std::shared_ptr<ShaderProgram> GPURegistry::getShader(AssetUID id, const std::shared_ptr<Shader> &shader_asset) {
    if (m_shader_programs.contains(id)) {
        return m_shader_programs[id];
    } else {
        auto program = m_graphics_factory->createShader(*shader_asset);
        m_shader_programs[id] = program;
        return program;
    }
}

std::shared_ptr<GPUMesh> GPURegistry::getMesh(AssetUID id, const std::shared_ptr<Mesh> &mesh_asset) {
    if (m_gpu_meshes.contains(id)) {
        return m_gpu_meshes[id];
    } else {
        auto vertexArray = m_graphics_factory->createVertexArray();

        auto vertexBuffer = m_graphics_factory->createVertexBuffer();
        auto normalsBuffer = m_graphics_factory->createVertexBuffer();
        auto uvBuffer = m_graphics_factory->createVertexBuffer();
        auto tangentBuffer = m_graphics_factory->createVertexBuffer();
        auto biTangentBuffer = m_graphics_factory->createVertexBuffer();
        auto boneIDBuffer = m_graphics_factory->createVertexBuffer();
        auto boneWeightBuffer = m_graphics_factory->createVertexBuffer();
        auto indexBuffer = m_graphics_factory->createIndexBuffer();

        auto data = mesh_asset->getMeshData();
        vertexBuffer->putData(BufferUtils::CreateFloatBuffer(data.vertices), 3 * data.vertices.size() * sizeof(float));
        normalsBuffer->putData(BufferUtils::CreateFloatBuffer(data.normals), 3 * data.normals.size() * sizeof(float));
        tangentBuffer->putData(BufferUtils::CreateFloatBuffer(data.tangents), 3 * data.tangents.size() * sizeof(float));
        biTangentBuffer->putData(BufferUtils::CreateFloatBuffer(data.bitangents), 3 * data.bitangents.size() * sizeof(float));
        boneIDBuffer->putData(BufferUtils::CreateIntBuffer(data.boneIDs), 4 * data.boneIDs.size() * sizeof(int));
        boneWeightBuffer->putData(BufferUtils::CreateFloatBuffer(data.boneWeights), 4 * data.boneWeights.size() * sizeof(float));
        uvBuffer->putData(BufferUtils::CreateFloatBuffer(data.uvCoords), 2 * data.uvCoords.size() * sizeof(float));
        indexBuffer->putData(BufferUtils::CreateIntBuffer(data.indices), 3 * data.indices.size() * sizeof(int));

        vertexArray->pushVertexBuffer(vertexBuffer, 0, 3);
        vertexArray->pushVertexBuffer(normalsBuffer, 1, 3);
        vertexArray->pushVertexBuffer(uvBuffer, 2, 2);
        vertexArray->pushVertexBuffer(tangentBuffer, 3, 3);
        vertexArray->pushVertexBuffer(biTangentBuffer, 4, 3);
        vertexArray->pushVertexBuffer(boneIDBuffer, 5, 4);
        vertexArray->pushVertexBuffer(boneWeightBuffer, 6, 4);
        vertexArray->setIndexBuffer(indexBuffer);

        std::shared_ptr<GPUMesh> gpu_mesh = std::make_shared<GPUMesh>(vertexArray);
        m_gpu_meshes[id] = gpu_mesh;
        return gpu_mesh;
    }
}

std::shared_ptr<GPUTexture> GPURegistry::getTexture2D(AssetUID id, const std::shared_ptr<Texture2D> &tex_asset) {
    if (m_gpu_tex2d.contains(id)) {
        return m_gpu_tex2d[id];
    } else {
        auto gpu_texture = m_graphics_factory->createTexture2D(*tex_asset);
        m_gpu_tex2d[id] = gpu_texture;
        return gpu_texture;
    }
}

std::shared_ptr<GPUTexture> GPURegistry::getCubemap(AssetUID id, const std::shared_ptr<TextureCube> &tex_asset) {
    if (m_gpu_cubemaps.contains(id)) {
        return m_gpu_cubemaps[id];
    } else {
        auto gpu_texture = m_graphics_factory->createTextureCube(*tex_asset);
        m_gpu_cubemaps[id] = gpu_texture;
        return gpu_texture;
    }
}
}  // namespace ICE
