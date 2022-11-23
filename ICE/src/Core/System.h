//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_SYSTEM_H
#define ICE_SYSTEM_H

#include <Scene/Scene.h>

namespace ICE {
    class System {
    public:
        virtual void update(Scene* scene, double delta) = 0;
        virtual void entitySignatureChanged(Entity e);
    };

    class SystemManager {

    };
}

#endif //ICE_SYSTEM_H
