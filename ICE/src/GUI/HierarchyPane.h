//
// Created by Thomas Ibanez on 29.11.20.
//

#ifndef ICE_HIERARCHYPANE_H
#define ICE_HIERARCHYPANE_H

#include <Scene/Scene.h>
#include "ICEPane.h"

namespace ICE {
    class ICEEngine;
    class HierarchyPane : public ICEPane {
    public:
        HierarchyPane(ICEEngine* engine);
        bool render() override;
    private:
        void mkPopup(const std::string parent);
        void subtree(Scene::SceneNode* node);
        ICEEngine* engine;
        std::string selected;
    };
}


#endif //ICE_HIERARCHYPANE_H
