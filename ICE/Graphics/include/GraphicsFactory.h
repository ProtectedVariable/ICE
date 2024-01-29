#pragma once

#include <GraphicsFactory.h>

#include <memory>

#include "Context.h"
#include "Framebuffer.h"
#include "GraphicsAPI.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include <Window.h>
#include <memory>

namespace ICE {
class GraphicsFactory {
   public:
    virtual std::shared_ptr<Context> createContext(const std::shared_ptr<Window> &window) = 0;

    virtual std::shared_ptr<Framebuffer> createFramebuffer(const FrameBufferFormat& format) = 0;

    virtual std::shared_ptr<RendererAPI> createRendererAPI() = 0;

    virtual std::shared_ptr<VertexArray> createVertexArray() = 0;

    virtual std::shared_ptr<VertexBuffer> createVertexBuffer() = 0;

    virtual std::shared_ptr<IndexBuffer> createIndexBuffer() = 0;

    virtual std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& fragmentFile) = 0;

    virtual std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& geometryFile, const std::string& fragmentFile) = 0;

    virtual std::shared_ptr<Texture2D> createTexture2D(const std::string& file) = 0;

    virtual std::shared_ptr<TextureCube> createTextureCube(const std::string& file) = 0;
};
}  // namespace ICE