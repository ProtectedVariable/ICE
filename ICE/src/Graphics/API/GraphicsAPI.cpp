//
// Created by Thomas Ibanez on 20.11.20.
//
#include "GraphicsAPI.h"

#include <Graphics/API/OpenGL/OpenGLRendererAPI.h>
#include <Graphics/API/OpenGL/OpenGLBuffers.h>

namespace ICE {

    RendererAPI* RendererAPI::Create() {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLRendererAPI();
        }
        return nullptr;
    }

    VertexBuffer* VertexBuffer::Create(uint32_t size) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLVertexBuffer();
        }
        return nullptr;
    }

    IndexBuffer* IndexBuffer::Create(uint32_t *indices, uint32_t size) {
        switch(RendererAPI::GetAPI()) {
            case OpenGL: return new OpenGLIndexBuffer();
        }
        return nullptr;
    }
}