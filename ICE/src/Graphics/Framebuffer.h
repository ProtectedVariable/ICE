//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_FRAMEBUFFER_H
#define ICE_FRAMEBUFFER_H
namespace ICE {
    struct FrameBufferFormat {
        int width, height, samples;
    };

    class Framebuffer {
    public:
        virtual void bind() = 0;
        virtual void unbind() = 0;
        virtual void resize(int width, int height) = 0;
        virtual void* getTexture() = 0;

        static Framebuffer* Create(FrameBufferFormat format);
    private:
        FrameBufferFormat format;
    };
}
#endif //ICE_FRAMEBUFFER_H
