//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLVertexArray.h"
#include <OpenGL/gl3.h>
#include <OpenGL/gl.h>

ICE::OpenGLVertexArray::OpenGLVertexArray() {
    glGenVertexArrays(1, &vaoID);
}

void ICE::OpenGLVertexArray::bind() const {
    glBindVertexArray(this->vaoID);
    //TODO: Better buffer representation
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void ICE::OpenGLVertexArray::unbind() const {
    glBindVertexArray(0);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(const ICE::VertexBuffer* buffer, int size) {
    this->pushVertexBuffer(buffer, cnt++, size);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(const ICE::VertexBuffer* buffer, int position, int size) {
    this->bind();
    buffer->bind();
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, size, GL_FLOAT, false, 0, 0);
}

void ICE::OpenGLVertexArray::setIndexBuffer(const ICE::IndexBuffer* buffer) {
    this->bind();
    buffer->bind();
    this->indexCount = buffer->getSize() / sizeof(int);
}

int ICE::OpenGLVertexArray::getIndexCount() const {
    return indexCount;
}

uint32_t ICE::OpenGLVertexArray::getID() const {
    return vaoID;
}
