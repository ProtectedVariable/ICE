//
// Created by Thomas Ibanez on 19.02.21.
//

#ifndef ICE_SCENEPARAMPANE_H
#define ICE_SCENEPARAMPANE_H

#include "ICEPane.h"

namespace ICE {
    class ICEEngine;

    class SceneParamPane : public ICEPane {
    public:
        bool render() override;
        SceneParamPane(ICEEngine* engine);
    private:
        ICEEngine* engine;
    };
}


#endif //ICE_SCENEPARAMPANE_H
