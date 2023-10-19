//
// Created by Thomas Ibanez on 19.11.20.
//

#ifndef ICE_RENDERSYSTEM_H
#define ICE_RENDERSYSTEM_H

#include <System.h>
#include <Renderer.h>
#include <Camera.h>
#include <Framebuffer.h>

namespace ICE {
    class Scene;

    class RenderSystem : public System {
    public:
        RenderSystem() {};

        void update(Scene* scene, double delta) override;

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
