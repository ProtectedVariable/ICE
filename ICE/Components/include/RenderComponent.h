//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_RENDERCOMPONENT_H
#define ICE_RENDERCOMPONENT_H

#include <Component.h>
#include <Material.h>
#include <Mesh.h>

namespace ICE {
struct RenderComponent : public Component {
    RenderComponent(AssetUID model_id) : model(model_id) {}
    AssetUID model;
};
}  // namespace ICE

#endif  //ICE_RENDERCOMPONENT_H
