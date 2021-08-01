//
// Created by Thomas Ibanez on 31.07.21.
//

#ifndef ICE_SHADERLOADER_H
#define ICE_SHADERLOADER_H

#include <string>
#include "IResourceLoader.h"
#include "Resource.h"

namespace ICE {
    class ShaderLoader : public IResourceLoader {
    public:
        Resource *load(const std::vector<std::string> &file) override;

    };
}


#endif //ICE_SHADERLOADER_H
