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

        AssetViewPane(ICEEngine *engine, std::string* selectedAsset);

    private:
        ICEEngine* engine;
        Framebuffer* viewFB;
        Camera camera;
        std::string* selectedAsset;
        ForwardRenderer renderer;
    };
}


#endif //ICE_ASSETVIEWPANE_H
