//
// Created by Thomas Ibanez on 25.11.20.
//

#ifndef ICE_ICEENGINE_H
#define ICE_ICEENGINE_H

#include <vector>
#include "System.h"

namespace ICE {
    class ICEEngine {
    public:
        ICEEngine(void* window);

        void initialize();
        void loop();
    private:
        std::vector<System*> systems;
        void* window;
        Scene* currentScene;
        RendererAPI* api;
        Context* ctx;
    };
}


#endif //ICE_ICEENGINE_H
