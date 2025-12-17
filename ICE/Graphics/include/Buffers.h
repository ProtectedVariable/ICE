//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_BUFFERS_H
#define ICE_BUFFERS_H

#include <cstdint>

namespace ICE {
class VertexBuffer {
   public:
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual void putData(const void* data, uint32_t size) = 0;
    virtual uint32_t getSize() const = 0;
};

class IndexBuffer {
   public:
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual void putData(const void* data, uint32_t size) = 0;
    virtual uint32_t getSize() const = 0;
};

class UniformBuffer {
   public:
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual void putData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
    virtual uint32_t getSize() const = 0;
    virtual ~UniformBuffer() = default;
};

}  // namespace ICE
#endif  //ICE_BUFFERS_H
