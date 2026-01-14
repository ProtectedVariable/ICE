#pragma once

#include <memory>

#include "Context.h"
#include "Framebuffer.h"
#include "GraphicsAPI.h"
#include "GraphicsFactory.h"
#include "OpenGLBuffers.h"
#include "OpenGLContext.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLRendererAPI.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"
#include "OpenGLVertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"

namespace ICE {
class OpenGLFactory : public GraphicsFactory {
   public:
    std::shared_ptr<Context> createContext(const std::shared_ptr<Window>& window) const override { return std::make_shared<OpenGLContext>(window); }

    std::shared_ptr<Framebuffer> createFramebuffer(const FrameBufferFormat& format) const override {
        return std::make_shared<OpenGLFramebuffer>(format);
    }

    std::shared_ptr<RendererAPI> createRendererAPI() const override { return std::make_shared<OpenGLRendererAPI>(); }

    std::shared_ptr<VertexArray> createVertexArray() const override { return std::make_shared<OpenGLVertexArray>(); }

    std::shared_ptr<VertexBuffer> createVertexBuffer() const override { return std::make_shared<OpenGLVertexBuffer>(); }

    std::shared_ptr<UniformBuffer> createUniformBuffer(size_t size, size_t binding) const override {
        return std::make_shared<OpenGLUniformBuffer>(size, binding);
    }

    std::shared_ptr<IndexBuffer> createIndexBuffer() const override { return std::make_shared<OpenGLIndexBuffer>(); }

    std::shared_ptr<ShaderProgram> createShader(const Shader& shader) const override { return std::make_shared<OpenGLShader>(shader); }

    std::shared_ptr<GPUTexture> createTexture2D(const Texture2D& texture) const override { return std::make_shared<OpenGLTexture2D>(texture); }
    std::shared_ptr<GPUTexture> createTexture2D(const void* data, size_t w, size_t h, TextureFormat fmt) const override {
        return std::make_shared<OpenGLTexture2D>(data, w, h, fmt);
    }

    std::shared_ptr<GPUTexture> createTextureCube(const TextureCube& texture) const override { return std::make_shared<OpenGLTextureCube>(texture); }
};
}  // namespace ICE