#pragma once

#include <GPUMesh.h>
#include <GPUTexture.h>
#include <GraphicsFactory.h>
#include <ShaderProgram.h>

#include <memory>

#include "Asset.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

namespace ICE {
class GPURegistry {
   public:
    GPURegistry(const std::shared_ptr<GraphicsFactory> &factory);

    std::shared_ptr<ShaderProgram> getShader(AssetUID id, const std::shared_ptr<Shader> &shader_asset);
    std::shared_ptr<GPUMesh> getMesh(AssetUID id, const std::shared_ptr<Mesh> &mesh_asset);
    std::shared_ptr<GPUTexture> getTexture2D(AssetUID id, const std::shared_ptr<Texture2D> &tex_asset);
    std::shared_ptr<GPUTexture> getCubemap(AssetUID id, const std::shared_ptr<TextureCube> &tex_asset);

   private:
    std::unordered_map<AssetUID, std::shared_ptr<ShaderProgram>> m_shader_programs;
    std::unordered_map<AssetUID, std::shared_ptr<GPUMesh>> m_gpu_meshes;
    std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> m_gpu_tex2d;
    std::unordered_map<AssetUID, std::shared_ptr<GPUTexture>> m_gpu_cubemaps;

    std::shared_ptr<GraphicsFactory> m_graphics_factory;
};
}  // namespace ICE
