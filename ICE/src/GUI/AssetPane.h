//
// Created by Thomas Ibanez on 03.12.20.
//

#ifndef ICE_ASSETPANE_H
#define ICE_ASSETPANE_H


#include "ICEPane.h"
#include "AssetViewPane.h"
#include "AssetContentPane.h"

namespace ICE {

    class AssetPane : public ICEPane {
    public:
        explicit AssetPane(ICEEngine *engine);

        void initialize() override;

        bool render() override;
    private:
        ICEEngine* engine;
        AssetViewPane viewPane;
        AssetContentPane contentPane;
        int selectedDirectory = 0;
        AssetUID selectedAsset = NO_ASSET_ID;
    };
}


#endif //ICE_ASSETPANE_H
