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
        RenderComponent(AssetUID mesh, AssetUID material, AssetUID shader);

        AssetUID getMesh() const;

        AssetUID getMaterial() const;

        void setMesh(AssetUID mesh);

        void setMaterial(AssetUID material);

        AssetUID getShader();

        void setShader(AssetUID shader);

    private:
        AssetUID mesh;
        AssetUID material;
        AssetUID shader;
    };
}

#endif //ICE_RENDERCOMPONENT_H
