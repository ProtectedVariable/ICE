//
// Created by Thomas Ibanez on 03.12.20.
//

#ifndef ICE_ASSETPANE_H
#define ICE_ASSETPANE_H

#include <Core/ICEEngine.h>
#include "ICEPane.h"
#include "AssetViewPane.h"
#include "AssetContentPane.h"

namespace ICE {
    class AssetPane : public ICEPane {
    public:
        AssetPane(ICEEngine *engine);

        void render() override;
    private:
        ICEEngine* engine;
        AssetViewPane* viewPane;
        AssetContentPane* contentPane;
        int selectedDirectory = 0;
        std::string selectedAsset = "";
    };
}


#endif //ICE_ASSETPANE_H
