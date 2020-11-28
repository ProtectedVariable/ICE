//
// Created by Thomas Ibanez on 27.11.20.
//

#ifndef ICE_ICEGUI_H
#define ICE_ICEGUI_H

#include <Graphics/Framebuffer.h>

namespace ICE {
    class ICEGUI {
    public:
        ICEGUI(Framebuffer *framebuffer);

        void renderImGui();
    private:
        Framebuffer* framebuffer;
    };
}


#endif //ICE_ICEGUI_H
