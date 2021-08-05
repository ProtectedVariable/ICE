//
// Created by Thomas Ibanez on 31.07.21.
//

#ifndef ICE_IRESOURCELOADER_H
#define ICE_IRESOURCELOADER_H

#include "Resource.h"

namespace ICE {
    class IResourceLoader {
    public:
        virtual Resource* load(const std::vector<std::string> &files) = 0;
    };
}

#endif //ICE_IRESOURCELOADER_H
