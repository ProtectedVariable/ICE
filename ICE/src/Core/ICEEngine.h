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
        Eigen::Vector4i getPickingTextureAt(int x, int y);

        Framebuffer *getInternalFB() const;

        Camera *getCamera();

        AssetBank* getAssetBank();

        Scene* getScene();

        Entity *getSelected() const;

        RendererAPI *getApi() const;

        void setSelected(Entity *selected);

    private:
        std::vector<System*> systems;
        void* window;
        Scene currentScene;
        RendererAPI* api;
        Context* ctx;
        ICEGUI* gui;
        Framebuffer* internalFB;
        Framebuffer* pickingFB;
        Camera camera;
        AssetBank assetBank;
        Entity* selected;
    };
}


#endif //ICE_ICEENGINE_H
