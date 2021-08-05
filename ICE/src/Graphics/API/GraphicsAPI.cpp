//
// Created by Thomas Ibanez on 20.11.20.
//
#include "GraphicsAPI.h"
#include <GLFW/glfw3.h>
#include <Graphics/API/OpenGL/OpenGLRendererAPI.h>
#include <Graphics/API/OpenGL/OpenGLBuffers.h>
#include <Graphics/Context.h>
#include <Graphics/API/OpenGL/OpenGLContext.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Shader.h>
#include <Graphics/API/OpenGL/OpenGLShader.h>
#include <Graphics/API/OpenGL/OpenGLVertexArray.h>
#include <Graphics/API/OpenGL/OpenGLFramebuffer.h>
#include <Graphics/Texture.h>
#include <Graphics/API/OpenGL/OpenGLTexture.h>
#include <Graphics/API/None/NoneGraphics.h>

namespace ICE {

    GraphicsAPI RendererAPI::api = OpenGL;

    Context* Context::Create(void* windowHandle) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLContext(static_cast<GLFWwindow*>(windowHandle));
            case None: return new NoneContext();
        }
        return nullptr;
    }

    Framebuffer* Framebuffer::Create(FrameBufferFormat format) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLFramebuffer(format);
            case None: return new NoneFramebuffer();
        }
    }

    RendererAPI* RendererAPI::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLRendererAPI();
            case None: return new NoneRendererAPI();
        }
    }

    VertexArray* VertexArray::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLVertexArray();
            case None: return new NoneVertexArray();
        }
    }

    VertexBuffer* VertexBuffer::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLVertexBuffer();
            case None: return new NoneVertexBuffer();
        }
    }

    IndexBuffer* IndexBuffer::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLIndexBuffer();
            case None: return new NoneIndexBuffer();
        }
    }

    Shader* Shader::Create(const std::string &vertexFile, const std::string &fragmentFile) {
        return Create(vertexFile, "", fragmentFile);
    }

    Shader* Shader::Create(const std::string &vertexFile, const std::string &geometryFile, const std::string &fragmentFile) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLShader(vertexFile, geometryFile, fragmentFile);
            case None: return new NoneShader();
        }
    }

    Texture2D* Texture2D::Create(const std::string& file) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLTexture2D(file);
            case None: return new NoneTexture2D();
        }
    }

    TextureCube* TextureCube::Create(const std::string &file) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLTextureCube(file);
            case None: return new NoneTextureCube();
        }
    }
}