//
// Created by Thomas Ibanez on 04.12.20.
//

#ifndef ICE_ASSETCONTENTPANE_H
#define ICE_ASSETCONTENTPANE_H


#include <Graphics/Framebuffer.h>
#include <Graphics/Camera.h>
#include "ICEPane.h"

#define ICE_THUMBNAIL_SIZE (64)
#define ICE_MAX_THUMBNAILS (128)

namespace ICE {
    class ICEEngine;

    class AssetContentPane : public ICEPane {
    public:
        void render() override;

        AssetContentPane(const int *selectedDir, ICEEngine *engine, std::string* selectedAsset);

    private:
        const int* selectedDir;
        ICEEngine* engine;
        Framebuffer* thumbnailFBO[ICE_MAX_THUMBNAILS];
        std::string* selectedAsset;
        Camera camera;
    };
}


#endif //ICE_ASSETCONTENTPANE_H
