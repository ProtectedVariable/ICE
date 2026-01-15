#pragma once

#include <GPUMesh.h>
#include <GPUTexture.h>
#include <GraphicsFactory.h>
#include <ShaderProgram.h>

#include <memory>

#include "Asset.h"
#include "AssetBank.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

namespace ICE {
class GPURegistry {
   public:
    GPURegistry(const std::shared_ptr<GraphicsFactory> &factory, const std::shared_ptr<AssetBank> &bank);

    AssetUID getUID(const AssetPath &path) const { return m_asset_bank->getUID(path); }
    std::shared_ptr<Material> getMaterial(const AssetPath &path) { return m_asset_bank->getAsset<Material>(getUID(path)); }
    std::shared_ptr<Material> getMaterial(AssetUID id) { return m_asset_bank->getAsset<Material>(id); }
    std::shared_ptr<ShaderProgram> getShader(AssetUID id);
    std::shared_ptr<ShaderProgram> getShader(const AssetPath &path) { return getShader(getUID(path)); }
    AABB getMeshAABB(AssetUID id) { return m_asset_bank->getAsset<Mesh>(id)->getBoundingBox(); }
    std::shared_ptr<GPUMesh> getMesh(AssetUID id);
    std::shared_ptr<GPUMesh> getMesh(const AssetPath &path) { return getMesh(getUID(path)); }
    std::shared_ptr<GPUTexture> getTexture2D(AssetUID id);
    std::shared_ptr<GPUTexture> getTexture2D(const AssetPath &path) { return getTexture2D(getUID(path)); }
    std::shared_ptr<GPUTexture> getCubemap(AssetUID id);

   private:
    std::unordered_map<AssetUID, std::shared_ptr<ShaderProgram>> m_shader_programs;
    std::unordered_map<AssetUID, std::shared_ptr<GPUMesh>> m_gpu_meshes;
    std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> m_gpu_tex2d;
    std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> m_gpu_cubemaps;

    std::shared_ptr<GraphicsFactory> m_graphics_factory;
    std::shared_ptr<AssetBank> m_asset_bank;
};
}  // namespace ICE
