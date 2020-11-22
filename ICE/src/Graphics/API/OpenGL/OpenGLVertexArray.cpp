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
}

void ICE::OpenGLVertexArray::unbind() const {
    glBindVertexArray(0);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(const ICE::VertexBuffer* buffer) {
    this->pushVertexBuffer(buffer, cnt++);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(const ICE::VertexBuffer* buffer, int position) {
    this->bind();
    buffer->bind();
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, buffer->getSize(), GL_FLOAT, false, 0, 0);
}

void ICE::OpenGLVertexArray::setIndexBuffer(const ICE::IndexBuffer* buffer) {
    this->bind();
    buffer->bind();
}

int ICE::OpenGLVertexArray::getIndexCount() const {
    return 0;
}

uint32_t ICE::OpenGLVertexArray::getID() const {
    return vaoID;
}
