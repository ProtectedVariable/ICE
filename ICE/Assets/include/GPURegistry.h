#pragma once

#include <GPUMesh.h>
#include <GPUTexture.h>
#include <GpuHandle.h>
#include <GraphicsFactory.h>
#include <HandlePool.h>
#include <ShaderProgram.h>

#include <memory>
#include <unordered_map>

#include "Asset.h"
#include "AssetBank.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

namespace ICE {
// Owns the GPU-side resources (meshes, 2D textures, shaders) uploaded from CPU assets. Ownership
// lives in generational HandlePools -- the registry is the single owner; everyone else holds a
// lightweight handle (resolved to a raw pointer at bind time) or, on the not-yet-migrated paths, a
// non-owning shared_ptr. The shared_ptr accessors keep their original signatures so existing
// callers are unchanged while the per-frame path migrates to handles (see resolve()).
class GPURegistry {
   public:
    GPURegistry(const std::shared_ptr<GraphicsFactory> &factory, const std::shared_ptr<AssetBank> &bank);
    ~GPURegistry();

    // Single owner of the GPU pools and holder of a self-referential removal-listener registration:
    // non-copyable (a copy would double-unregister and not own its own listener).
    GPURegistry(const GPURegistry &) = delete;
    GPURegistry &operator=(const GPURegistry &) = delete;

    // Release every GPU resource uploaded from `id` (mesh / 2D texture / shader / cubemap) and drop
    // the by-uid entries. Invoked via the AssetBank removal listener when an asset is removed or
    // re-imported. Outstanding handles held by in-flight render jobs go stale (resolve() -> nullptr,
    // safe by design); the next frame's Phase 1 re-resolves and lazily re-uploads if the asset
    // still exists.
    void evict(AssetUID id);

    AssetUID getUID(const AssetPath &path) const { return m_asset_bank->getUID(path); }
    std::shared_ptr<Material> getMaterial(const AssetPath &path) { return m_asset_bank->getAsset<Material>(getUID(path)); }
    std::shared_ptr<Material> getMaterial(AssetUID id) { return m_asset_bank->getAsset<Material>(id); }
    std::shared_ptr<ShaderProgram> getShader(AssetUID id);
    std::shared_ptr<ShaderProgram> getShader(const AssetPath &path) { return getShader(getUID(path)); }
    // Null-safe: an evicted/removed mesh yields a zero AABB instead of dereferencing null. Callers on
    // the frame path resolve the mesh handle first and discard the job when it is gone, so this
    // default is a backstop rather than a rendered value.
    AABB getMeshAABB(AssetUID id) {
        auto mesh = m_asset_bank->getAsset<Mesh>(id);
        return mesh ? mesh->getBoundingBox() : AABB{Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()};
    }
    const SkinningData &getMeshSkinningData(AssetUID id) { return m_asset_bank->getAsset<Mesh>(id)->getSkinningData(); }
    std::shared_ptr<GPUMesh> getMesh(AssetUID id);
    std::shared_ptr<GPUMesh> getMesh(const AssetPath &path) { return getMesh(getUID(path)); }
    std::shared_ptr<GPUTexture> getTexture2D(AssetUID id);
    std::shared_ptr<GPUTexture> getTexture2D(const AssetPath &path) { return getTexture2D(getUID(path)); }
    std::shared_ptr<GPUTexture> getCubemap(AssetUID id);

    // Handle API (the per-frame path migrates onto this): *Handle() uploads the resource if needed
    // and returns a generational handle; resolve() turns a handle into a raw pointer at bind time,
    // or nullptr if the resource was freed/reuploaded (a stale handle). No shared_ptr, no refcount
    // churn, no per-frame texture-map copies.
    MeshHandle meshHandle(AssetUID id);
    TextureHandle textureHandle(AssetUID id);
    ShaderHandle shaderHandle(AssetUID id);
    MeshHandle meshHandle(const AssetPath &path) { return meshHandle(getUID(path)); }
    TextureHandle textureHandle(const AssetPath &path) { return textureHandle(getUID(path)); }
    ShaderHandle shaderHandle(const AssetPath &path) { return shaderHandle(getUID(path)); }

    // Ensure a 2D texture is resident and return a raw pointer to it (nullptr if it isn't a valid
    // texture asset). Used by the geometry pass to bind a material's textures without a shared_ptr.
    GPUTexture *texture2DPtr(AssetUID id) { return resolve(textureHandle(id)); }

    // --- Residency stats -------------------------------------------------------------------------
    // Measurement foundation for a future eviction policy: expose how much is resident so memory
    // pressure can be measured before any budget/LRU policy is enforced (see the eviction task,
    // deliberately profiling-gated). None of these touch the per-frame resolve path.
    //
    // Live GPU resources per pool (cheap; safe to poll every frame).
    std::size_t residentMeshCount() const { return m_mesh_pool.size(); }
    std::size_t residentTextureCount() const { return m_tex2d_pool.size(); }
    std::size_t residentShaderCount() const { return m_shader_pool.size(); }

    // Estimated bytes of the CPU payload backing the resident meshes / 2D textures. Reads the source
    // assets, so it is O(resident) and intended for occasional diagnostics, not per frame. Textures
    // are estimated at 4 bytes/texel.
    std::size_t residentMeshBytes();
    std::size_t residentTextureBytes();

    GPUMesh *resolve(MeshHandle h) {
        auto *sp = m_mesh_pool.get(h);
        return sp ? sp->get() : nullptr;
    }
    GPUTexture *resolve(TextureHandle h) {
        auto *sp = m_tex2d_pool.get(h);
        return sp ? sp->get() : nullptr;
    }
    ShaderProgram *resolve(ShaderHandle h) {
        auto *sp = m_shader_pool.get(h);
        return sp ? sp->get() : nullptr;
    }

   private:
    // Single-owner pools for the hot-path resources; the by-uid maps translate an AssetUID to its
    // handle so the shared_ptr accessors and the handle accessors share one upload.
    HandlePool<std::shared_ptr<GPUMesh>, GPUMesh> m_mesh_pool;
    HandlePool<std::shared_ptr<GPUTexture>, GPUTexture> m_tex2d_pool;
    HandlePool<std::shared_ptr<ShaderProgram>, ShaderProgram> m_shader_pool;
    std::unordered_map<AssetUID, MeshHandle> m_mesh_by_uid;
    std::unordered_map<AssetUID, TextureHandle> m_tex2d_by_uid;
    std::unordered_map<AssetUID, ShaderHandle> m_shader_by_uid;

    // Cubemaps stay a plain map: skybox is not the per-frame hot path (one per scene).
    std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> m_gpu_cubemaps;

    std::shared_ptr<GraphicsFactory> m_graphics_factory;
    std::shared_ptr<AssetBank> m_asset_bank;
    AssetBank::RemovalListenerHandle m_removal_listener_handle = 0;
};
}  // namespace ICE
