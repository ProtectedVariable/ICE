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

namespace ICE {

    Context* Context::Create(void* windowHandle) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLContext(static_cast<GLFWwindow*>(windowHandle));
        }
        return nullptr;
    }

    Framebuffer* Framebuffer::Create(FrameBufferFormat format) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLFramebuffer(format);
        }
        return nullptr;
    }

    RendererAPI* RendererAPI::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLRendererAPI();
        }
        return nullptr;
    }

    VertexArray* VertexArray::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLVertexArray();
        }
        return nullptr;
    }

    VertexBuffer* VertexBuffer::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLVertexBuffer();
        }
        return nullptr;
    }

    IndexBuffer* IndexBuffer::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLIndexBuffer();
        }
        return nullptr;
    }

    Shader* Shader::Create(const std::string &vertexFile, const std::string &fragmentFile) {
        return Create(vertexFile, "", fragmentFile);
    }

    Shader* Shader::Create(const std::string &vertexFile, const std::string &geometryFile, const std::string &fragmentFile) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLShader(vertexFile, geometryFile, fragmentFile);
        }
        return nullptr;
    }
}