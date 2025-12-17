#include "Model.h"

namespace ICE {
Model::Model(const std::vector<Node> &nodes, const std::vector<std::shared_ptr<ICE::Mesh>> &meshes, const std::vector<ICE::AssetUID> &materials)
    : m_nodes(nodes),
      m_meshes(meshes),
      m_materials(materials) {
    for (const auto &mesh : meshes) {
        m_boundingbox = m_boundingbox.unionWith(mesh->getBoundingBox());
    }
}

}  // namespace ICE