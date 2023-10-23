//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_VERTEXARRAY_H
#define ICE_VERTEXARRAY_H

#include <cstdint>
#include <memory>

#include "Buffers.h"

namespace ICE {
class VertexBuffer;
class IndexBuffer;

class VertexArray {
   public:
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual void pushVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, int size) = 0;
    virtual void pushVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, int position, int size) = 0;
    virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer) = 0;
    virtual std::shared_ptr<IndexBuffer> getIndexBuffer() const = 0;
    virtual int getIndexCount() const = 0;
    virtual uint32_t getID() const = 0;
};
}  // namespace ICE

#endif  //ICE_VERTEXARRAY_H
