//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_RENDERSYSTEM_H
#define ICE_RENDERSYSTEM_H

#include <Core/System.h>

namespace ICE {
    class RenderSystem : public System {
        void update(Scene scene, double delta) override;

    };
}


#endif //ICE_RENDERSYSTEM_H
