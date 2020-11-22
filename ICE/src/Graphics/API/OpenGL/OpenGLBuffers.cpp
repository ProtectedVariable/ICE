//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLBuffers.h"
#include <OpenGL/gl3.h>

ICE::OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) : size(size) {
    glGenBuffers(1, &id);
}

void ICE::OpenGLIndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

void ICE::OpenGLIndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //not sure this makes sense
}

uint32_t ICE::OpenGLIndexBuffer::getSize() const {
    return this->size;
}

void ICE::OpenGLIndexBuffer::putData(const void *data, uint32_t size) {
    this->bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    this->size = size;
    this->unbind();
}

ICE::OpenGLIndexBuffer::OpenGLIndexBuffer() {
    glGenBuffers(1, &id);
}

void ICE::OpenGLVertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
}

void ICE::OpenGLVertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ICE::OpenGLVertexBuffer::putData(const void *data, uint32_t size) {
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    this->size = size;
    this->unbind();
}

uint32_t ICE::OpenGLVertexBuffer::getSize() const {
    return this->size;
}
