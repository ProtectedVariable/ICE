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

        int getSceneViewportWidth() const;

        int getSceneViewportHeight() const;

    private:
        Framebuffer* framebuffer;
        int init = 0;
        int sceneViewportWidth, sceneViewportHeight;
    };
}


#endif //ICE_ICEGUI_H
