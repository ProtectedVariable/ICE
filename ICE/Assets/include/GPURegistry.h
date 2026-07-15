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

    AssetUID getUID(const AssetPath &path) const { return m_asset_bank->getUID(path); }
    std::shared_ptr<Material> getMaterial(const AssetPath &path) { return m_asset_bank->getAsset<Material>(getUID(path)); }
    std::shared_ptr<Material> getMaterial(AssetUID id) { return m_asset_bank->getAsset<Material>(id); }
    std::shared_ptr<ShaderProgram> getShader(AssetUID id);
    std::shared_ptr<ShaderProgram> getShader(const AssetPath &path) { return getShader(getUID(path)); }
    AABB getMeshAABB(AssetUID id) { return m_asset_bank->getAsset<Mesh>(id)->getBoundingBox(); }
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
};
}  // namespace ICE
