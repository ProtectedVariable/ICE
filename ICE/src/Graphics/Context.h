//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_CONTEXT_H
#define ICE_CONTEXT_H
namespace ICE {
    class Context {
    public:
        virtual void initialize() = 0;
        virtual void swapBuffers() = 0;
        virtual void wireframeMode() = 0;
        virtual void fillMode() = 0;

        static Context* Create(void* windowHandle);
    };
}
#endif //ICE_CONTEXT_H
