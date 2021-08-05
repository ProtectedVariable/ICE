//
// Created by Thomas Ibanez on 30.11.20.
//

#ifndef ICE_INSPECTORPANE_H
#define ICE_INSPECTORPANE_H

#include <GUI/Components/UIComponentRenderer.h>
#include "ICEPane.h"

namespace ICE {
    class ICEEngine;

    class InspectorPane : public ICEPane {
    public:
        bool render() override;

        void initialize() override;

        InspectorPane(ICEEngine* engine);
    private:
        ICEEngine* engine;
        UIComponentRenderer componentRenderer;
        int init = 0;
    };
}


#endif //ICE_INSPECTORPANE_H
