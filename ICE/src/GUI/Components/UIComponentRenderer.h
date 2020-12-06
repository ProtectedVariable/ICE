//
// Created by Thomas Ibanez on 30.11.20.
//

#ifndef ICE_UICOMPONENTRENDERER_H
#define ICE_UICOMPONENTRENDERER_H
#define FLOAT_MAX_DIGITS 128
#include <Scene/TransformComponent.h>
#include <Scene/RenderComponent.h>
#include <unordered_map>
#include <Scene/LightComponent.h>

namespace ICE {
    class ICEEngine;

    class UIComponentRenderer {
    public:
        void render(TransformComponent* cmp);
        void render(RenderComponent* cmp);
        void render(LightComponent* lc);

        UIComponentRenderer(ICEEngine *engine);

    private:
        void renderVector3f(Eigen::Vector3f* vec);
        ICEEngine* engine;
    };
}


#endif //ICE_UICOMPONENTRENDERER_H
