//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLVERTEXARRAY_H
#define ICE_OPENGLVERTEXARRAY_H

#include <Graphics/VertexArray.h>
#include <OpenGL/gl.h>

namespace ICE {
    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray();

        void bind() override;

        void unbind() override;

        void pushVertexBuffer(const VertexBuffer &buffer) override;

        void pushVertexBuffer(const VertexBuffer &buffer, int position) override;

        void setIndexBuffer(const IndexBuffer &buffer) override;

        int getIndexCount() override;

        uint32_t getID() override;

    private:
        GLuint vaoID;
        int cnt = 0;
    };
}


#endif //ICE_OPENGLVERTEXARRAY_H
