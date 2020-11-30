//
// Created by Thomas Ibanez on 25.11.20.
//

#ifndef ICE_ICEENGINE_H
#define ICE_ICEENGINE_H

#include <GL/gl3w.h>
#include <vector>
#include <GUI/ICEGUI.h>
#include <Assets/AssetBank.h>
#include "System.h"

namespace ICE {
    class ICEEngine {
    public:
        ICEEngine(void* window);

        void initialize();
        void loop();

        Framebuffer *getInternalFb() const;

        Camera *getCamera() const;

        AssetBank *getAssetBank() const;

        Scene *getScene() const;

        Entity *getSelected() const;

        void setSelected(Entity *selected);

    private:
        std::vector<System*> systems;
        void* window;
        Scene* currentScene;
        RendererAPI* api;
        Context* ctx;
        ICEGUI* gui;
        Framebuffer* internalFB;
        Camera *camera;
        AssetBank* assetBank;
        Entity* selected;
    };
}


#endif //ICE_ICEENGINE_H
