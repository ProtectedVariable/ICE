//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_RENDERCOMPONENT_H
#define ICE_RENDERCOMPONENT_H

#include <Component.h>
#include <Mesh.h>
#include <Material.h>

namespace ICE {
    struct RenderComponent : public Component {
        AssetUID mesh;
        AssetUID material;
        AssetUID shader;
    };
}

#endif //ICE_RENDERCOMPONENT_H
