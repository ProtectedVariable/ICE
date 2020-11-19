//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_SYSTEM_H
#define ICE_SYSTEM_H

#include <Scene/Scene.h>

namespace ICE {
    class System {
        virtual void update(Scene scene, double delta) = 0;
    };
}

#endif //ICE_SYSTEM_H
