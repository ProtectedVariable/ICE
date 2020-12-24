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
        void edit(const std::string& name, Material& material);
        NewMaterialPane(ICEEngine* engine);

    private:
        ICEEngine* engine;
        Eigen::Vector3f albedo, specular, ambient;
        float alpha;
        const Texture* diffuseMap;
        const Texture* specularMap;
        const Texture* ambientMap;
        const Texture* normalMap;
        Framebuffer* viewFB;
        Camera camera;
        float y = 0;
        std::string name, oldname;
        bool editMode = false;
    };
}


#endif //ICE_NEWMATERIALPANE_H
