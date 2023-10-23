//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLVERTEXARRAY_H
#define ICE_OPENGLVERTEXARRAY_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <VertexArray.h>
#include <vector>
#include <unordered_map>

namespace ICE {
    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray();

        void bind() const override;

        void unbind() const override;

        void pushVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, int size) override;

        void pushVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, int position, int size) override;

        void setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer) override;

        int getIndexCount() const override;

        uint32_t getID() const override;

        std::shared_ptr<IndexBuffer> getIndexBuffer() const override;

    private:
        GLuint vaoID;
        int cnt = 0;
        GLuint indexCount = 0;
        std::unordered_map<GLuint, std::shared_ptr<VertexBuffer>> buffers;
        std::shared_ptr<IndexBuffer> indexBuffer;
    };
}


#endif //ICE_OPENGLVERTEXARRAY_H
