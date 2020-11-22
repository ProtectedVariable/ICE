//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLBUFFERS_H
#define ICE_OPENGLBUFFERS_H

#include <Graphics/Buffers.h>
#include <OpenGL/gl.h>

namespace ICE {

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        void bind() const override;

        void unbind() const override;

        void putData(const void *data, uint32_t size) override;

        uint32_t getSize() const override;

        uint32_t getComponents() const;

        OpenGLVertexBuffer(): OpenGLVertexBuffer(0) {}

        OpenGLVertexBuffer(uint32_t size);

    private:
        GLuint id;
        uint32_t size;
    };

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        void bind() const override;

        void unbind() const override;

        uint32_t getSize() const override;

        void putData(const void *data, uint32_t size) override;

        OpenGLIndexBuffer();

    private:
        GLuint id;
        uint32_t size;
    };
}


#endif //ICE_OPENGLBUFFERS_H
