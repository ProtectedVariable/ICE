//
// Created by Thomas Ibanez on 21.12.20.
//

#ifndef ICE_NEWMATERIALPANE_H
#define ICE_NEWMATERIALPANE_H


#include <Graphics/Material.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Camera.h>
#include "ICEPane.h"

namespace ICE {
    class ICEEngine;

    class NewMaterialPane : ICEPane {
    public:
        bool render() override;
        void build();
        void reset();
        NewMaterialPane(ICEEngine* engine);

    private:
        ICEEngine* engine;
        Eigen::Vector3f albedo, specular, ambient;
        float alpha;
        Framebuffer* viewFB;
        Camera camera;
        float y = 0;
        std::string name;
    };
}


#endif //ICE_NEWMATERIALPANE_H
