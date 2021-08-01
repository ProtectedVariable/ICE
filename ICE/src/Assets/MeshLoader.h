//
// Created by Thomas Ibanez on 31.07.21.
//

#ifndef ICE_MESHLOADER_H
#define ICE_MESHLOADER_H


#include <string>
#include "IResourceLoader.h"
#include "Resource.h"

namespace ICE {
    class MeshLoader: public IResourceLoader {
    public:
        Resource *load(const std::vector<std::string> &file) override;
    };
}


#endif //ICE_MESHLOADER_H
