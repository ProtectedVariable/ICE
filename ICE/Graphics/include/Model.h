#pragma once

#include "Asset.h"
#include "Mesh.h"

namespace ICE {
class Model : public Asset {
   public:
    Model(const std::vector<std::shared_ptr<ICE::Mesh>> &meshes, const std::vector<ICE::AssetUID> &materials);

    std::vector<std::shared_ptr<ICE::Mesh>> getMeshes() const { return m_meshes; }
    std::vector<ICE::AssetUID> getMaterialsIDs() const { return m_materials; }
    AABB getBoundingBox() const { return m_boundingbox; }

    AssetType getType() const override { return AssetType::EModel; }
    std::string getTypeName() const override { return "Model"; }

   private:
    std::vector<std::shared_ptr<ICE::Mesh>> m_meshes;
    std::vector<ICE::AssetUID> m_materials;
    AABB m_boundingbox{{0, 0, 0}, {0, 0, 0}};
};
}  // namespace ICE