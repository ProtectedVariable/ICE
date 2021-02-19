//
// Created by Thomas Ibanez on 29.11.20.
//

#ifndef ICE_ASSETBANK_H
#define ICE_ASSETBANK_H

#include <unordered_map>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>
#include <Graphics/Texture.h>

#define ICE_ASSET_PREFIX "__ice__"

namespace ICE {
    class AssetBank {
    public:
        AssetBank();

        void fillWithDefaults();

        bool addMesh(const std::string& name, Mesh* mesh);
        bool addMaterial(const std::string& name, Material* mtl);
        bool addShader(const std::string& name, Shader* shader);
        bool addTexture(const std::string& name, Texture* texture);

        bool renameAsset(const std::string& oldName, const std::string& newName);

        Mesh* getMesh(const std::string& name);
        Material* getMaterial(const std::string& name);
        Shader* getShader(const std::string& name);
        Texture* getTexture(const std::string& name);

        const std::unordered_map<std::string, Mesh*> &getMeshes() const;
        const std::unordered_map<std::string, Material*> &getMaterials() const;
        const std::unordered_map<std::string, Shader*> &getShaders() const;
        const std::unordered_map<std::string, Texture*> &getTextures() const;

        std::string getName(const void* ptr);
        bool nameInUse(const std::string& name);

    private:
        std::unordered_map<std::string, Mesh*> meshes;
        std::unordered_map<std::string, Material*> materials;
        std::unordered_map<std::string, Shader*> shaders;
        std::unordered_map<std::string, Texture*> textures;
    };
}


#endif //ICE_ASSETBANK_H
