//
// Created by Thomas Ibanez on 22.11.20.
//

#ifndef ICE_LIGHTCOMPONENT_H
#define ICE_LIGHTCOMPONENT_H

#include <Eigen/Dense>
#include "Component.h"

namespace ICE {
    enum LightType {
        PointLight,
        DirectionalLight,
        SpotLight
    };

    class LightComponent : public Component {
    public:
        LightComponent(LightType type, Eigen::Vector3f color);

        LightType getType() const;

        void setType(LightType type);

        Eigen::Vector3f &getColor();

        void setColor(const Eigen::Vector3f &color);

    private:
        LightType type;
        Eigen::Vector3f color;
    };
}


#endif //ICE_LIGHTCOMPONENT_H
