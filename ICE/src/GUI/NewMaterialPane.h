//
// Created by Thomas Ibanez on 21.12.20.
//

#ifndef ICE_NEWMATERIALPANE_H
#define ICE_NEWMATERIALPANE_H


#include "ICEPane.h"
#include <Graphics/Material.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Camera.h>
#include <Graphics/ForwardRenderer.h>

namespace ICE {
    class ICEEngine;

    class NewMaterialPane : ICEPane {
    public:
        bool render() override;
        void build();
        void reset();
        void edit(AssetUID selectedAsset, Material& material);
        NewMaterialPane(ICEEngine* engine);
        void initialize() override;

    private:
        Material makeMaterial();

        ICEEngine* engine;
        Eigen::Vector3f albedo, specular, ambient;
        float alpha;
        AssetUID diffuseMap;
        AssetUID specularMap;
        AssetUID ambientMap;
        AssetUID normalMap;
        Framebuffer* viewFB;
        Camera camera;
        float y = 0;
        std::string name, oldname;
        bool editMode = false;
        ForwardRenderer renderer;
        bool canceled = false;
    };
}


#endif //ICE_NEWMATERIALPANE_H
