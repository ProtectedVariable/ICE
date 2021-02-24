//
// Created by Thomas Ibanez on 24.02.21.
//

#ifndef ICE_NONEGRAPHICS_H
#define ICE_NONEGRAPHICS_H

#include <Graphics/Buffers.h>

namespace ICE {

    class NoneVertexBuffer : public VertexBuffer {
    public:
        void bind() const override {}
        void unbind() const override {}
        void putData(const void *data, uint32_t size) override {}
        uint32_t getSize() const override {}
        NoneVertexBuffer() {}
    };

    class NoneIndexBuffer : public IndexBuffer {
    public:
        void bind() const override {}
        void unbind() const override {}
        uint32_t getSize() const override {}
        void putData(const void *data, uint32_t size) override {}
        NoneIndexBuffer() {}
    };

    class NoneContext : public Context {
    public:
        void swapBuffers() override {}
        void wireframeMode() override {}
        void fillMode() override {}
        void initialize() override {}
        NoneContext() {}
    };
}


#endif //ICE_NONEGRAPHICS_H
