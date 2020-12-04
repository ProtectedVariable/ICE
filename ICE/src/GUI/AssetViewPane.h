//
// Created by Thomas Ibanez on 04.12.20.
//

#ifndef ICE_ASSETVIEWPANE_H
#define ICE_ASSETVIEWPANE_H

#include <Graphics/Framebuffer.h>
#include <Core/ICEEngine.h>
#include "ICEPane.h"

namespace ICE {
    class AssetViewPane : public ICEPane {
    public:
        void render() override;

        AssetViewPane(ICEEngine *engine, std::string* selectedAsset);

    private:
        ICEEngine* engine;
        Framebuffer* viewFB;
        Camera* camera;
        std::string* selectedAsset;
    };
}


#endif //ICE_ASSETVIEWPANE_H
