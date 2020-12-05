//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_RENDERCOMPONENT_H
#define ICE_RENDERCOMPONENT_H

#include <Scene/Component.h>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>

namespace ICE {
    class RenderComponent : public Component {
    public:
        RenderComponent(Mesh* mesh, Material* material);

        Mesh* getMesh() const;

        Material* getMaterial() const;

        void setMesh(Mesh *mesh);

        void setMaterial(Material *material);

    private:
        Mesh* mesh;
        Material* material;
    };
}

#endif //ICE_RENDERCOMPONENT_H
