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

        void bind() const override;

        void unbind() const override;

        void pushVertexBuffer(const VertexBuffer* buffer) override;

        void pushVertexBuffer(const VertexBuffer* buffer, int position) override;

        void setIndexBuffer(const IndexBuffer* buffer) override;

        int getIndexCount() const override;

        uint32_t getID() const override;

    private:
        GLuint vaoID;
        int cnt = 0;
    };
}


#endif //ICE_OPENGLVERTEXARRAY_H
