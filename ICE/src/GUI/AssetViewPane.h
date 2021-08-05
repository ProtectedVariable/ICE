//
// Created by Thomas Ibanez on 04.12.20.
//

#ifndef ICE_ASSETVIEWPANE_H
#define ICE_ASSETVIEWPANE_H

#include <Graphics/Framebuffer.h>
#include <Graphics/Camera.h>
#include <Graphics/ForwardRenderer.h>
#include "ICEPane.h"

namespace ICE {
    class ICEEngine;
    class AssetViewPane : public ICEPane {
    public:
        bool render() override;

        AssetViewPane(ICEEngine *engine, AssetUID* selectedAsset);

        void initialize() override;

    private:
        ICEEngine* engine;
        Framebuffer* viewFB;
        Camera camera;
        AssetUID* selectedAsset;
        ForwardRenderer renderer;
    };
}


#endif //ICE_ASSETVIEWPANE_H
