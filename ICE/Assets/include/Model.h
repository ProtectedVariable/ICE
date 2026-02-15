#pragma once

#include "AABB.h"
#include "Animation.h"
#include "Asset.h"
#include "Mesh.h"

namespace ICE {
class Model : public Asset {
   public:
    struct Node {
        std::string name;
        Eigen::Matrix4f localTransform;   // default transform
        std::vector<size_t> meshIndices;  // Meshes used by this node
        std::vector<int> children;        // Node indices
    };

    struct Skeleton {
        std::unordered_map<std::string, int> boneMapping;
        Eigen::Matrix4f globalInverseTransform;
    };

    Model(const std::vector<Node> &nodes, const std::vector<AssetUID> &meshes, const std::vector<ICE::AssetUID> &materials);

    std::vector<Node> getNodes() const { return m_nodes; }
    std::vector<Node> &getNodes() { return m_nodes; }
    std::vector<AssetUID> getMeshes() const { return m_meshes; }
    std::vector<AssetUID> getMaterialsIDs() const { return m_materials; }
    AABB getBoundingBox() const { return m_boundingbox; }
    std::unordered_map<std::string, Animation> getAnimations() const { return m_animations; }
    Skeleton &getSkeleton() { return m_skeleton; }
    void setSkeleton(const Skeleton &skeleton) { m_skeleton = skeleton; }
    void setAnimations(const std::unordered_map<std::string, Animation> &animations) { m_animations = animations; }

    void traverse(std::vector<AssetUID> &meshes, std::vector<AssetUID> &materials, std::vector<Eigen::Matrix4f> &transforms,
                  const Eigen::Matrix4f &base_transform = Eigen::Matrix4f::Identity());

    AssetType getType() const override { return AssetType::EModel; }
    std::string getTypeName() const override { return "Model"; }

   private:
    std::vector<Node> m_nodes;
    std::vector<AssetUID> m_meshes;
    std::vector<AssetUID> m_materials;
    std::unordered_map<std::string, Animation> m_animations;
    Skeleton m_skeleton;
    AABB m_boundingbox{{0, 0, 0}, {0, 0, 0}};
};
}  // namespace ICE