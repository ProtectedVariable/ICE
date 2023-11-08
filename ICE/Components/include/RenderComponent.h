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
    RenderComponent(AssetUID mesh_id, AssetUID material_id) : mesh(mesh_id), material(material_id) {}
    AssetUID mesh;
    AssetUID material;
};
}  // namespace ICE

#endif  //ICE_RENDERCOMPONENT_H
