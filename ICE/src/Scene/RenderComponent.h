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
        RenderComponent(const Mesh* mesh, const Material* material, Shader* shader);

        const Mesh* getMesh() const;

        const Material* getMaterial() const;

        void setMesh(Mesh *mesh);

        void setMaterial(Material *material);

        Shader *getShader();

        void setShader(Shader *shader);

    private:
        const Mesh* mesh;
        const Material* material;
        Shader* shader;
    };
}

#endif //ICE_RENDERCOMPONENT_H
