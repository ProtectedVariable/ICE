//
// Created by Thomas Ibanez on 29.07.21.
//

#ifndef ICE_RESOURCE_H
#define ICE_RESOURCE_H

#include "Asset.h"
#include <vector>
#include <string>
namespace ICE {
    class Resource {
    public:
        Asset* asset;
        const std::vector<std::string> source;

        Resource(Asset *asset, const std::vector<std::string> &source) : asset(asset), source(source) {}
    };
}

#endif //ICE_RESOURCE_H
