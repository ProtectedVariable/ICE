//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_CONTEXT_H
#define ICE_CONTEXT_H
namespace ICE {
    class Context {
    public:
        static Context* Create() {

        }
        virtual void Initialize() = 0;
        virtual void SwapBuffers() = 0;
        virtual void WireframeMode() = 0;
        virtual void FillMode() = 0;
    };
}
#endif //ICE_CONTEXT_H
