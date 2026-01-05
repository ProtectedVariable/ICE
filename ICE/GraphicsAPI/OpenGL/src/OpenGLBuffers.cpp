//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLBuffers.h"

#include <GL/gl3w.h>
#include <cassert>

namespace ICE {

/////////////////////////////////// INDEX BUFFER //////////////////////////////////

void OpenGLIndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

void OpenGLIndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  //not sure this makes sense
}

uint32_t OpenGLIndexBuffer::getSize() const {
    return this->size;
}

void OpenGLIndexBuffer::putData(const void *data, uint32_t size) {
    this->bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    this->size = size;
    this->unbind();
}

OpenGLIndexBuffer::OpenGLIndexBuffer() {
    glGenBuffers(1, &id);
}

/////////////////////////////////// VERTEX BUFFER //////////////////////////////////

OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) : size(size) {
    glGenBuffers(1, &id);
}

void OpenGLVertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
}

void OpenGLVertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLVertexBuffer::putData(const void *data, uint32_t size) {
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    this->size = size;
    this->unbind();
}

uint32_t OpenGLVertexBuffer::getSize() const {
    return this->size;
}

////////////////////////////////// UNIFORM BUFFER //////////////////////////////////

OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t _size, uint32_t _binding) : size(_size), binding(_binding) {
    glGenBuffers(1, &id);
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, id);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

OpenGLUniformBuffer::~OpenGLUniformBuffer() {
    glDeleteBuffers(1, &id);
}

void OpenGLUniformBuffer::bind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, id);
}

uint32_t OpenGLUniformBuffer::getSize() const {
    return size;
}

void OpenGLUniformBuffer::unbind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, id);
}

void OpenGLUniformBuffer::putData(const void *data, uint32_t _size, uint32_t offset) {
    assert(offset + _size <= size);
    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, _size, data);
    unbind();
}
}  // namespace ICE
