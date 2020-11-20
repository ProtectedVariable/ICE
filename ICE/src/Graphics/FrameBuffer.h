//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_FRAMEBUFFER_H
#define ICE_FRAMEBUFFER_H
namespace ICE {
    struct FrameBufferFormat {
        int width, height, samples;
    };

    class FrameBuffer {
    public:
        virtual void bind() = 0;
        virtual void unbind() = 0;
        virtual void resize(int width, int height) = 0;

        static FrameBuffer* Create(FrameBufferFormat format);
    private:
        FrameBufferFormat format;
    };
}
#endif //ICE_FRAMEBUFFER_H
