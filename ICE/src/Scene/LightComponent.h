//
// Created by Thomas Ibanez on 22.11.20.
//

#ifndef ICE_LIGHTCOMPONENT_H
#define ICE_LIGHTCOMPONENT_H

#include "Component.h"

namespace ICE {
    enum LightType {
        PointLight,
        DirectionalLight,
        SpotLight
    };

    class LightComponent : public Component {
    public:
        LightComponent(LightType type);

        LightType getType() const;

        void setType(LightType type);

    private:
        LightType type;
    };
}


#endif //ICE_LIGHTCOMPONENT_H
