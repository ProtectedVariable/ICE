//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_VERTEXARRAY_H
#define ICE_VERTEXARRAY_H

#include "Buffer.h"

namespace ICE {
    class VertexArray {
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual void PushVertexBuffer(const VertexBuffer &buffer) = 0;
        virtual void PushVertexBuffer(const VertexBuffer &buffer, int position) = 0;
        virtual void SetIndexBuffer(const IndexBuffer &buffer) = 0;
        static VertexArray* Create();
    };
}

#endif //ICE_VERTEXARRAY_H
