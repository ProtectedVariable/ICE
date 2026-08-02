#include "GPURegistry.h"

namespace ICE {
GPURegistry::GPURegistry(const std::shared_ptr<GraphicsFactory> &factory, const std::shared_ptr<AssetBank> &bank)
    : m_graphics_factory(factory),
      m_asset_bank(bank) {
    // Subscribe to asset removals: without this, the by-uid maps would pin every upload forever, so
    // removing an asset leaks its GPU memory and re-importing keeps rendering the old data.
    m_removal_listener_handle = m_asset_bank->addRemovalListener([this](AssetUID id) { evict(id); });
}

GPURegistry::~GPURegistry() {
    // The bank can outlive the registry (the registry holds a shared_ptr to it), so a listener that
    // captures `this` must be removed before we are destroyed.
    if (m_asset_bank) {
        m_asset_bank->removeRemovalListener(m_removal_listener_handle);
    }
}

std::size_t GPURegistry::residentMeshBytes() {
    std::size_t total = 0;
    for (const auto &[uid, handle] : m_mesh_by_uid) {
        auto mesh = m_asset_bank->getAsset<Mesh>(uid);
        if (!mesh) {
            continue;
        }
        const auto &d = mesh->getMeshData();
        total += d.vertices.size() * sizeof(Eigen::Vector3f);
        total += d.normals.size() * sizeof(Eigen::Vector3f);
        total += d.uvCoords.size() * sizeof(Eigen::Vector2f);
        total += d.tangents.size() * sizeof(Eigen::Vector3f);
        total += d.bitangents.size() * sizeof(Eigen::Vector3f);
        total += d.boneIDs.size() * sizeof(Eigen::Vector4i);
        total += d.boneWeights.size() * sizeof(Eigen::Vector4f);
        total += d.indices.size() * sizeof(Eigen::Vector3i);
    }
    return total;
}

std::size_t GPURegistry::residentTextureBytes() {
    std::size_t total = 0;
    for (const auto &[uid, handle] : m_tex2d_by_uid) {
        auto tex = m_asset_bank->getAsset<Texture2D>(uid);
        if (!tex) {
            continue;
        }
        total += static_cast<std::size_t>(tex->getWidth()) * static_cast<std::size_t>(tex->getHeight()) * 4;
    }
    return total;
}

void GPURegistry::evict(AssetUID id) {
    if (auto it = m_mesh_by_uid.find(id); it != m_mesh_by_uid.end()) {
        m_mesh_pool.erase(it->second);
        m_mesh_by_uid.erase(it);
    }
    if (auto it = m_tex2d_by_uid.find(id); it != m_tex2d_by_uid.end()) {
        m_tex2d_pool.erase(it->second);
        m_tex2d_by_uid.erase(it);
    }
    if (auto it = m_shader_by_uid.find(id); it != m_shader_by_uid.end()) {
        m_shader_pool.erase(it->second);
        m_shader_by_uid.erase(it);
    }
    m_gpu_cubemaps.erase(id);
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
