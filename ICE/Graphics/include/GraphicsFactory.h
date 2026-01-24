#pragma once

#include <GraphicsFactory.h>
#include <Window.h>

#include <memory>

#include "Context.h"
#include "Framebuffer.h"
#include "GraphicsAPI.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include "GPUTexture.h"
#include "Texture.h"
#include "VertexArray.h"

namespace ICE {
class GraphicsFactory {
   public:
    virtual std::shared_ptr<Context> createContext(const std::shared_ptr<Window>& window) const = 0;

    virtual std::shared_ptr<Framebuffer> createFramebuffer(const FrameBufferFormat& format) const = 0;

    virtual std::shared_ptr<RendererAPI> createRendererAPI() const = 0;

    virtual std::shared_ptr<VertexArray> createVertexArray() const = 0;

    virtual std::shared_ptr<VertexBuffer> createVertexBuffer() const = 0;

    virtual std::shared_ptr<IndexBuffer> createIndexBuffer() const = 0;

    virtual std::shared_ptr<UniformBuffer> createUniformBuffer(size_t size, size_t binding) const = 0;

    virtual std::shared_ptr<ShaderProgram> createShader(const Shader& shader) const = 0;

    virtual std::shared_ptr<GPUTexture> createTexture2D(const Texture2D &texture) const = 0;

    virtual std::shared_ptr<GPUTexture> createTextureCube(const TextureCube& texture) const = 0;
};
}  // namespace ICE