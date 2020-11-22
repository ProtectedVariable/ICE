//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_RENDERERCONFIG_H
#define ICE_RENDERERCONFIG_H

namespace ICE {
    struct RendererConfig {
        int instancingThreshold; //Threshold after which entities sharing the same mesh & shader will be rendered using instancing, set to 0 for no instancing

    };
}

#endif //ICE_RENDERERCONFIG_H
