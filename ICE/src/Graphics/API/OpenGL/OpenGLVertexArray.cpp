//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLVertexArray.h"
#include <OpenGL/gl3.h>
#include <OpenGL/gl.h>

ICE::OpenGLVertexArray::OpenGLVertexArray(): buffers(std::unordered_map<GLuint, VertexBuffer*>()) {
    glGenVertexArrays(1, &vaoID);
}

void ICE::OpenGLVertexArray::bind() const {
    glBindVertexArray(this->vaoID);
}

void ICE::OpenGLVertexArray::unbind() const {
    glBindVertexArray(0);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(ICE::VertexBuffer* buffer, int size) {
    this->pushVertexBuffer(buffer, cnt, size);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(ICE::VertexBuffer* buffer, int position, int size) {
    this->bind();
    buffer->bind();
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, size, GL_FLOAT, false, 0, nullptr);
    this->buffers[position] = buffer;
    cnt = (position + 1) > cnt ? position + 1 : cnt;
}

void ICE::OpenGLVertexArray::setIndexBuffer(ICE::IndexBuffer* buffer) {
    this->bind();
    buffer->bind();
    this->indexCount = buffer->getSize() / sizeof(int);
    this->indexBuffer = buffer;
}

int ICE::OpenGLVertexArray::getIndexCount() const {
    return indexCount;
}

uint32_t ICE::OpenGLVertexArray::getID() const {
    return vaoID;
}
