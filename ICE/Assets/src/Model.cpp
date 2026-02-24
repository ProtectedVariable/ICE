#include "Model.h"

namespace ICE {
Model::Model(const std::vector<Node> &nodes, const std::vector<AssetUID> &meshes, const std::vector<AssetUID> &materials)
    : m_nodes(nodes),
      m_meshes(meshes),
      m_materials(materials) {
    /* for (const auto &mesh : meshes) {
        m_boundingbox = m_boundingbox.unionWith(mesh->getBoundingBox());
    }*/
}

void Model::traverse(std::vector<AssetUID> &meshes, std::vector<AssetUID> &materials, std::vector<Eigen::Matrix4f> &transforms,
                     const Eigen::Matrix4f &base_transform) {
    std::function<void(int, const Eigen::Matrix4f &)> recursive_traversal = [&](int node_idx, const Eigen::Matrix4f &transform) {
        auto &node = m_nodes.at(node_idx);
        for (const auto &i : node.meshIndices) {
            if (i >= m_meshes.size()) {
                continue;
            }
            auto mtl_id = m_materials.at(i);
            auto mesh = m_meshes.at(i);

            Eigen::Matrix4f node_transform;
            node_transform = transform * node.localTransform;

            meshes.push_back(mesh);
            materials.push_back(mtl_id);
            transforms.push_back(node_transform);
        }

        for (const auto &child_idx : node.children) {
            recursive_traversal(child_idx, transform);
        }
    };

    recursive_traversal(0, base_transform);
}

void Model::buildNodeNameMap() {
    if (m_nodeNameMapBuilt) return;
    for (int i = 0; i < static_cast<int>(m_nodes.size()); ++i) {
        m_nodeNameMap[m_nodes[i].name] = i;
    }
    m_nodeNameMapBuilt = true;
}

const Model::Node* Model::getNodeByName(const std::string &name) {
    buildNodeNameMap();
    auto it = m_nodeNameMap.find(name);
    if (it != m_nodeNameMap.end()) {
        return &m_nodes[it->second];
    }
    return nullptr;
}

}  // namespace ICE