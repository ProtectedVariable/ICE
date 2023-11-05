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
    std::shared_ptr<Context> createContext(const std::shared_ptr<Window> &window) { return std::make_shared<OpenGLContext>(window); }

    std::shared_ptr<Framebuffer> createFramebuffer(const FrameBufferFormat& format) { return std::make_shared<OpenGLFramebuffer>(format); }

    std::shared_ptr<RendererAPI> createRendererAPI() { return std::make_shared<OpenGLRendererAPI>(); }

    std::shared_ptr<VertexArray> createVertexArray() { return std::make_shared<OpenGLVertexArray>(); }

    std::shared_ptr<VertexBuffer> createVertexBuffer() { return std::make_shared<OpenGLVertexBuffer>(); }

    std::shared_ptr<IndexBuffer> createIndexBuffer() { return std::make_shared<OpenGLIndexBuffer>(); }

    std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& fragmentFile) { return createShader(vertexFile, "", fragmentFile); }

    std::shared_ptr<Shader> createShader(const std::string& vertexFile, const std::string& geometryFile, const std::string& fragmentFile) {
        return std::make_shared<OpenGLShader>(vertexFile, geometryFile, fragmentFile);
    }

    std::shared_ptr<Texture2D> createTexture2D(const std::string& file) { return std::make_shared<OpenGLTexture2D>(file); }

    std::shared_ptr<TextureCube> createTextureCube(const std::string& file) { return std::make_shared<OpenGLTextureCube>(file); }
};
}  // namespace ICE