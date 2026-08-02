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
    virtual ~GraphicsFactory() = default;
    virtual std::shared_ptr<Context> createContext(const std::shared_ptr<Window>& window) const = 0;

    virtual std::shared_ptr<Framebuffer> createFramebuffer(const FrameBufferFormat& format) const = 0;

    virtual std::shared_ptr<RendererAPI> createRendererAPI() const = 0;

    virtual std::shared_ptr<VertexArray> createVertexArray() const = 0;

    virtual std::shared_ptr<VertexBuffer> createVertexBuffer() const = 0;

    virtual std::shared_ptr<IndexBuffer> createIndexBuffer() const = 0;

    virtual std::shared_ptr<UniformBuffer> createUniformBuffer(size_t size, size_t binding) const = 0;

    virtual std::shared_ptr<ShaderProgram> createShader(const Shader& shader) const = 0;

    virtual std::shared_ptr<GPUTexture> createTexture2D(const Texture2D &texture) const = 0;

    // Create an empty GPU texture: storage only, no CPU data. This is what the render graph uses to
    // allocate a transient Texture2D resource from its descriptor (the overload above can only
    // upload an already-loaded asset). Not pure: a backend that hasn't implemented it returns null
    // and the graph leaves the resource virtual rather than failing to build.
    virtual std::shared_ptr<GPUTexture> createTexture2D(uint32_t width, uint32_t height, TextureFormat format) const {
        return nullptr;
    }

    virtual std::shared_ptr<GPUTexture> createTextureCube(const TextureCube& texture) const = 0;
};
}  // namespace ICE