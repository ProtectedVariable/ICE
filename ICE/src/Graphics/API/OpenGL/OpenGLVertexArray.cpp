//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLVertexArray.h"
#include <OpenGL/gl3.h>
#include <OpenGL/gl.h>

void ICE::OpenGLVertexArray::bind() {
    glBindVertexArray(this->vaoID);
}

void ICE::OpenGLVertexArray::unbind() {
    glBindVertexArray(0);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(const ICE::VertexBuffer &buffer) {
    this->pushVertexBuffer(buffer, cnt++);
}

void ICE::OpenGLVertexArray::pushVertexBuffer(const ICE::VertexBuffer &buffer, int position) {
    this->bind();
    buffer.bind();
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, buffer.getSize(), GL_FLOAT, false, 0, 0);
}

void ICE::OpenGLVertexArray::setIndexBuffer(const ICE::IndexBuffer &buffer) {
    this->bind();
    buffer.bind();
}

int ICE::OpenGLVertexArray::getIndexCount() {
    return 0;
}

ICE::OpenGLVertexArray::OpenGLVertexArray() {
    glGenVertexArrays(1, &vaoID);
}

uint32_t ICE::OpenGLVertexArray::getID() {
    return vaoID;
}
