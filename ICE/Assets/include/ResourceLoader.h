//
// Created by Thomas Ibanez on 29.07.21.
//

#ifndef ICE_RESOURCELOADER_H
#define ICE_RESOURCELOADER_H

#include "Resource.h"
#include "IResourceLoader.h"
#include <unordered_map>
#include <typeindex>
#include <Util/ICEException.h>

namespace ICE {
    class ResourceLoader {
    public:

        template<typename T>
        Resource* LoadResource(const std::vector<std::string> files) {
            if(loaders.find(typeid(T)) != loaders.end()) {
                return loaders[typeid(T)]->load(files);
            }
            throw ICEException();
        }

        template<typename T>
        void AddLoader(IResourceLoader* loader) {
            loaders[typeid(T)] = loader;
        }
    private:
        std::unordered_map<std::type_index, IResourceLoader*> loaders;
    };
}

#endif //ICE_RESOURCELOADER_H
