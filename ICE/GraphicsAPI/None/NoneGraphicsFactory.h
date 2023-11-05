#pragma once

#include <memory>

#include "Context.h"
#include "Framebuffer.h"
#include "GraphicsAPI.h"
#include "GraphicsFactory.h"
#include "NoneGraphics.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"

namespace ICE {
class NoneGraphicsFactory : public GraphicsFactory {
   public:
    std::shared_ptr<Context> createContext(const std::shared_ptr<Window> &window) { return std::make_shared<NoneContext>(); }

    std::shared_ptr<Framebuffer> createFramebuffer(const FrameBufferFormat& format) { return std::make_shared<NoneFramebuffer>(); }

    std::shared_ptr<RendererAPI> createRendererAPI() { return std::make_shared<NoneRendererAPI>(); }

    std::shared_ptr<VertexArray> createVertexArray() { return std::make_shared<NoneVertexArray>(); }

    std::shared_ptr<VertexBuffer> createVertexBuffer() { return std::make_shared<NoneVertexBuffer>(); }

    std::shared_ptr<IndexBuffer> createIndexBuffer() { return std::make_shared<NoneIndexBuffer>(); }

    std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& fragmentFile) { return createShader(vertexFile, "", fragmentFile); }

    std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& geometryFile, const std::string& fragmentFile) {
        return std::make_shared<NoneShader>();
    }

    std::shared_ptr<Texture2D> createTexture2D(const std::string& file) { return std::make_shared<NoneTexture2D>(); }

    std::shared_ptr<TextureCube> createTextureCube(const std::string& file) { return std::make_shared<NoneTextureCube>(); }
};
}  // namespace ICE