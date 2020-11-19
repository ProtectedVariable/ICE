//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_RENDERERPARAMS_H
#define ICE_RENDERERPARAMS_H

namespace ICE {
    struct RendererParams {
        int instancingThreshold; //Threshold after which entities sharing the same mesh & shader will be rendered using instancing, set to 0 for no instancing

    };
}

#endif //ICE_RENDERERPARAMS_H
