#include "GPURegistry.h"

namespace ICE {
GPURegistry::GPURegistry(const std::shared_ptr<GraphicsFactory> &factory, const std::shared_ptr<AssetBank> &bank)
    : m_graphics_factory(factory),
      m_asset_bank(bank) {
}

std::shared_ptr<ShaderProgram> GPURegistry::getShader(AssetUID id) {
    if (auto it = m_shader_by_uid.find(id); it != m_shader_by_uid.end()) {
        if (auto *sp = m_shader_pool.get(it->second)) {
            return *sp;
        }
    }
    auto shader_asset = m_asset_bank->getAsset<Shader>(id);
    if (!shader_asset) {
        return nullptr;
    }
    auto program = m_graphics_factory->createShader(*shader_asset);
    m_shader_by_uid[id] = m_shader_pool.insert(program);
    return program;
}

std::shared_ptr<GPUMesh> GPURegistry::getMesh(AssetUID id) {
    if (auto it = m_mesh_by_uid.find(id); it != m_mesh_by_uid.end()) {
        if (auto *sp = m_mesh_pool.get(it->second)) {
            return *sp;
        }
    }
    auto mesh_asset = m_asset_bank->getAsset<Mesh>(id);
    if (!mesh_asset) {
        return nullptr;
    }

    auto vertexArray = m_graphics_factory->createVertexArray();

    auto vertexBuffer = m_graphics_factory->createVertexBuffer();
    auto normalsBuffer = m_graphics_factory->createVertexBuffer();
    auto uvBuffer = m_graphics_factory->createVertexBuffer();
    auto tangentBuffer = m_graphics_factory->createVertexBuffer();
    auto biTangentBuffer = m_graphics_factory->createVertexBuffer();
    auto boneIDBuffer = m_graphics_factory->createVertexBuffer();
    auto boneWeightBuffer = m_graphics_factory->createVertexBuffer();
    auto indexBuffer = m_graphics_factory->createIndexBuffer();

    // Fixed-size Eigen vectors are tightly packed (e.g. sizeof(Vector3f) == 3 floats),
    // so a std::vector of them is a contiguous float/int array we can upload directly
    // -- no intermediate malloc'd staging buffers (which were previously leaked).
    const auto &data = mesh_asset->getMeshData();
    vertexBuffer->putData(data.vertices.data(), 3 * data.vertices.size() * sizeof(float));
    normalsBuffer->putData(data.normals.data(), 3 * data.normals.size() * sizeof(float));
    tangentBuffer->putData(data.tangents.data(), 3 * data.tangents.size() * sizeof(float));
    biTangentBuffer->putData(data.bitangents.data(), 3 * data.bitangents.size() * sizeof(float));
    boneIDBuffer->putData(data.boneIDs.data(), 4 * data.boneIDs.size() * sizeof(int));
    boneWeightBuffer->putData(data.boneWeights.data(), 4 * data.boneWeights.size() * sizeof(float));
    uvBuffer->putData(data.uvCoords.data(), 2 * data.uvCoords.size() * sizeof(float));
    indexBuffer->putData(data.indices.data(), 3 * data.indices.size() * sizeof(int));

    vertexArray->pushVertexBuffer(vertexBuffer, 0, 3);
    vertexArray->pushVertexBuffer(normalsBuffer, 1, 3);
    vertexArray->pushVertexBuffer(uvBuffer, 2, 2);
    vertexArray->pushVertexBuffer(tangentBuffer, 3, 3);
    vertexArray->pushVertexBuffer(biTangentBuffer, 4, 3);
    vertexArray->pushVertexBuffer(boneIDBuffer, 5, 4);
    vertexArray->pushVertexBuffer(boneWeightBuffer, 6, 4);
    vertexArray->setIndexBuffer(indexBuffer);

    std::shared_ptr<GPUMesh> gpu_mesh = std::make_shared<GPUMesh>(vertexArray);
    m_mesh_by_uid[id] = m_mesh_pool.insert(gpu_mesh);
    return gpu_mesh;
}

std::shared_ptr<GPUTexture> GPURegistry::getTexture2D(AssetUID id) {
    if (auto it = m_tex2d_by_uid.find(id); it != m_tex2d_by_uid.end()) {
        if (auto *sp = m_tex2d_pool.get(it->second)) {
            return *sp;
        }
    }
    auto tex_asset = m_asset_bank->getAsset<Texture2D>(id);
    if (!tex_asset) {
        return nullptr;
    }

    auto gpu_texture = m_graphics_factory->createTexture2D(*tex_asset);
    m_tex2d_by_uid[id] = m_tex2d_pool.insert(gpu_texture);
    return gpu_texture;
}

std::shared_ptr<GPUTexture> GPURegistry::getCubemap(AssetUID id) {
    if (m_gpu_cubemaps.contains(id)) {
        return m_gpu_cubemaps[id];
    } else {
        auto tex_asset = m_asset_bank->getAsset<TextureCube>(id);
        if (!tex_asset) {
            return nullptr;
        }
        auto gpu_texture = m_graphics_factory->createTextureCube(*tex_asset);
        m_gpu_cubemaps[id] = gpu_texture;
        return gpu_texture;
    }
}

MeshHandle GPURegistry::meshHandle(AssetUID id) {
    getMesh(id);  // ensure resident (uploads on first use)
    auto it = m_mesh_by_uid.find(id);
    return it != m_mesh_by_uid.end() ? it->second : MeshHandle{};
}

TextureHandle GPURegistry::textureHandle(AssetUID id) {
    getTexture2D(id);
    auto it = m_tex2d_by_uid.find(id);
    return it != m_tex2d_by_uid.end() ? it->second : TextureHandle{};
}

ShaderHandle GPURegistry::shaderHandle(AssetUID id) {
    getShader(id);
    auto it = m_shader_by_uid.find(id);
    return it != m_shader_by_uid.end() ? it->second : ShaderHandle{};
}
}  // namespace ICE
