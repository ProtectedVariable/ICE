//
// Created by Thomas Ibanez on 04.12.20.
//

#ifndef ICE_ASSETCONTENTPANE_H
#define ICE_ASSETCONTENTPANE_H


#include <Core/ICEEngine.h>
#include "ICEPane.h"

namespace ICE {
    class AssetContentPane : public ICEPane {
    public:
        void render() override;

        AssetContentPane(const int *selectedDir, ICEEngine *engine);

    private:
        const int* selectedDir;
        ICEEngine* engine;
    };
}


#endif //ICE_ASSETCONTENTPANE_H
