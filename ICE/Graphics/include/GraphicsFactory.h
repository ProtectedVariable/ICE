#pragma once

#include <GraphicsFactory.h>
#include <Window.h>

#include <memory>

#include "Context.h"
#include "Framebuffer.h"
#include "GraphicsAPI.h"
#include "Shader.h"
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

    virtual std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& fragmentFile) const = 0;

    virtual std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& geometryFile,
                                                 const std::string& fragmentFile) const = 0;

    virtual std::shared_ptr<Texture2D> createTexture2D(const std::string& file) const = 0;
    virtual std::shared_ptr<Texture2D> createTexture2D(const void* data, size_t w, size_t h, TextureFormat fmt) const = 0;

    virtual std::shared_ptr<TextureCube> createTextureCube(const std::string& file) const = 0;
};
}  // namespace ICE