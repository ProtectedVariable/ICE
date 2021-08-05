//
// Created by Thomas Ibanez on 01.08.21.
//

#ifndef ICE_MATERIALLOADER_H
#define ICE_MATERIALLOADER_H

#include "IResourceLoader.h"

namespace ICE {
    class MaterialLoader : public IResourceLoader {
    public:
        Resource *load(const std::vector<std::string> &files) override;
    };
}


#endif //ICE_MATERIALLOADER_H
