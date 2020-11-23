//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLVERTEXARRAY_H
#define ICE_OPENGLVERTEXARRAY_H

#include <Graphics/VertexArray.h>
#include <OpenGL/gl.h>
#include <vector>
#include <unordered_map>

namespace ICE {
    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray();

        void bind() const override;

        void unbind() const override;

        void pushVertexBuffer(VertexBuffer* buffer, int size) override;

        void pushVertexBuffer(VertexBuffer* buffer, int position, int size) override;

        void setIndexBuffer(IndexBuffer* buffer) override;

        int getIndexCount() const override;

        uint32_t getID() const override;

    private:
        GLuint vaoID;
        int cnt = 0;
        GLuint indexCount;
        std::unordered_map<GLuint, VertexBuffer*> buffers;
        IndexBuffer* indexBuffer;
    };
}


#endif //ICE_OPENGLVERTEXARRAY_H
