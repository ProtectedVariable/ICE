//
// Created by Thomas Ibanez on 20.11.20.
//
#include "GraphicsAPI.h"
#include <GLFW/glfw3.h>
#include <OpenGLRendererAPI.h>
#include <OpenGLBuffers.h>
#include <Context.h>
#include <OpenGLContext.h>
#include <Framebuffer.h>
#include <Shader.h>
#include <OpenGLShader.h>
#include <OpenGLVertexArray.h>
#include <OpenGLFramebuffer.h>
#include <Texture.h>
#include <OpenGLTexture.h>
#include <NoneGraphics.h>

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