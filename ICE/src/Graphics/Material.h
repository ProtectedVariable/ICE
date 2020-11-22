//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_MATERIAL_H
#define ICE_MATERIAL_H

#include "Shader.h"

namespace ICE {
    class Material {
    public:
        Material(Shader *shader);

        Shader *getShader() const;

    private:
        Shader* shader;
    };
}




#endif //ICE_MATERIAL_H
