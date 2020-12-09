//
// Created by Thomas Ibanez on 29.11.20.
//

#ifndef ICE_ASSETBANK_H
#define ICE_ASSETBANK_H

#include <unordered_map>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>
#define ICE_ASSET_PREFIX "__ice__"

namespace ICE {
    class AssetBank {
    public:
        AssetBank();

        bool addMesh(const std::string& name, Mesh* mesh);
        bool addMaterial(const std::string& name, Material* mtl);
        bool addShader(const std::string& name, Shader* shader);


        Mesh* getMesh(const std::string& name);
        Material* getMaterial(const std::string& name);
        Shader* getShader(const std::string& name);

        const std::unordered_map<std::string, Mesh *> &getMeshes() const;

        const std::unordered_map<std::string, Material *> &getMaterials() const;

        const std::unordered_map<std::string, Shader *> &getShaders() const;

    private:
        std::unordered_map<std::string, Mesh*> meshes;
        std::unordered_map<std::string, Material*> materials;
        std::unordered_map<std::string, Shader*> shaders;
    };
}


#endif //ICE_ASSETBANK_H
