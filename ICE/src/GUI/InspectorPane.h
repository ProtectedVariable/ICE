//
// Created by Thomas Ibanez on 30.11.20.
//

#ifndef ICE_INSPECTORPANE_H
#define ICE_INSPECTORPANE_H

#include <Core/ICEEngine.h>
#include <GUI/Components/UIComponentRenderer.h>
#include "ICEPane.h"

namespace ICE {
    class InspectorPane : public ICEPane {
    public:
        void render() override;
        InspectorPane(ICEEngine* engine);
    private:
        ICEEngine* engine;
        UIComponentRenderer componentRenderer;
    };
}


#endif //ICE_INSPECTORPANE_H
