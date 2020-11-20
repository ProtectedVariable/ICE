//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_VERTEXARRAY_H
#define ICE_VERTEXARRAY_H

#include <cstdint>
#include "Buffers.h"

namespace ICE {
    class VertexBuffer;
    class IndexBuffer;

    class VertexArray {
    public:
        virtual void bind() = 0;
        virtual void unbind() = 0;
        virtual void pushVertexBuffer(const VertexBuffer &buffer) = 0;
        virtual void pushVertexBuffer(const VertexBuffer &buffer, int position) = 0;
        virtual void setIndexBuffer(const IndexBuffer &buffer) = 0;
        virtual int getIndexCount() = 0;
        virtual uint32_t getID() = 0;

        static VertexArray* Create() {
            return nullptr;
        }
    };
}

#endif //ICE_VERTEXARRAY_H
