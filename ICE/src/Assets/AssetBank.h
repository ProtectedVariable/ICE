//
// Created by Thomas Ibanez on 29.11.20.
//

#ifndef ICE_ASSETBANK_H
#define ICE_ASSETBANK_H

#include <unordered_map>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>

namespace ICE {
    class AssetBank {
    public:
        AssetBank();

        Mesh* getMesh(const std::string& name);
        Material* getMaterial(const std::string& name);
    private:
        std::unordered_map<std::string, Mesh*> meshes;
        std::unordered_map<std::string, Material*> materials;
        std::unordered_map<std::string, Shader*> shaders;
    };
}


#endif //ICE_ASSETBANK_H
