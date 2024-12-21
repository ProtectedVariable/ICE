#include "Model.h"

namespace ICE {
Model::Model(const std::vector<std::shared_ptr<ICE::Mesh>> &meshes, const std::vector<ICE::AssetUID> &materials)
    : m_meshes(meshes),
      m_materials(materials) {
}

}  // namespace ICE