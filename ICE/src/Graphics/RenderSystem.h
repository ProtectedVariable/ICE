//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_RENDERSYSTEM_H
#define ICE_RENDERSYSTEM_H

#include <Core/System.h>

namespace ICE {
    class RenderSystem : public System {
    public:
        RenderSystem(Renderer *renderer, Camera* camera);

        void update(Scene* scene, double delta) override;
        void entitySignatureChanged(Entity e) override;

        Renderer *getRenderer() const;
        void setRenderer(Renderer *renderer);
        Camera *getCamera() const;
        void setCamera(Camera *camera);

        void setTarget(Framebuffer* fb, int width, int height);

    private:
        Renderer* renderer;
        Camera* camera;
    };
}


#endif //ICE_RENDERSYSTEM_H
