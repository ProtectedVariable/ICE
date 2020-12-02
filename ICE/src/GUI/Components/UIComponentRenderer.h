//
// Created by Thomas Ibanez on 30.11.20.
//

#ifndef ICE_UICOMPONENTRENDERER_H
#define ICE_UICOMPONENTRENDERER_H
#define FLOAT_MAX_DIGITS 128
#include <Scene/TransformComponent.h>
#include <Scene/RenderComponent.h>

namespace ICE {
    class UIComponentRenderer {
    public:
        void render(TransformComponent* cmp);
        void render(RenderComponent* cmp);

    private:
        void renderVector3f(Eigen::Vector3f* vec);
    };
}


#endif //ICE_UICOMPONENTRENDERER_H
