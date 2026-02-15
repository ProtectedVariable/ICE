//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLVertexArray.h"

#include <GL/gl3w.h>

#include <iostream>

namespace ICE {
OpenGLVertexArray::OpenGLVertexArray() {
    glGenVertexArrays(1, &vaoID);
}

void OpenGLVertexArray::bind() const {
    glBindVertexArray(this->vaoID);
}

void OpenGLVertexArray::unbind() const {
    glBindVertexArray(0);
}

void OpenGLVertexArray::pushVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, int size) {
    this->pushVertexBuffer(buffer, cnt, size);
}

void OpenGLVertexArray::pushVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, int position, int size) {
    this->bind();
    buffer->bind();
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, size, GL_FLOAT, false, 0, 0);
    this->buffers[position] = buffer;
    cnt = (position + 1) > cnt ? position + 1 : cnt;
}

void OpenGLVertexArray::pushVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, int position, int size, int divisor) {
    this->bind();
    buffer->bind();
    
    // For mat4, we need 4 vec4 attributes
    if (size == 16) {  // mat4 = 4 * vec4
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(position + i);
            glVertexAttribPointer(position + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 4 * i));
            glVertexAttribDivisor(position + i, divisor);
        }
        cnt = (position + 4) > cnt ? position + 4 : cnt;
    } else {
        glEnableVertexAttribArray(position);
        glVertexAttribPointer(position, size, GL_FLOAT, false, 0, 0);
        glVertexAttribDivisor(position, divisor);
        cnt = (position + 1) > cnt ? position + 1 : cnt;
    }
    
    this->buffers[position] = buffer;
}

void OpenGLVertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer) {
    this->bind();
    buffer->bind();
    this->indexCount = buffer->getSize() / (sizeof(int));
    this->indexBuffer = buffer;
}

int OpenGLVertexArray::getIndexCount() const {
    return indexCount;
}

uint32_t OpenGLVertexArray::getID() const {
    return vaoID;
}

std::shared_ptr<IndexBuffer> OpenGLVertexArray::getIndexBuffer() const {
    return this->indexBuffer;
}
}  // namespace ICE
