#pragma once

#include "Animation.h"
#include "Asset.h"
#include "Mesh.h"

namespace ICE {
class Model : public Asset {
   public:
    struct Node {
        std::string name;
        Eigen::Matrix4f localTransform;
        Eigen::Matrix4f globalTransform;  // For mesh-only animation (no bones)
        std::vector<size_t> meshIndices;  // Meshes used by this node
        std::vector<int> children;        // Node indices
    };

    struct BoneInfo {
        Eigen::Matrix4f offsetMatrix;
        Eigen::Matrix4f finalTransformation;
    };

    struct Skeleton {
        std::unordered_map<std::string, int> boneMapping;
        std::vector<BoneInfo> bones;
        Eigen::Matrix4f globalInverseTransform;
    };

    Model(const std::vector<Node> &nodes, const std::vector<std::shared_ptr<ICE::Mesh>> &meshes, const std::vector<ICE::AssetUID> &materials);

    std::vector<Node> getNodes() const { return m_nodes; }
    std::vector<Node>& getNodes() { return m_nodes; }
    std::vector<std::shared_ptr<ICE::Mesh>> getMeshes() const { return m_meshes; }
    std::vector<ICE::AssetUID> getMaterialsIDs() const { return m_materials; }
    AABB getBoundingBox() const { return m_boundingbox; }
    std::unordered_map<std::string, Animation> getAnimations() const { return m_animations; }
    Skeleton &getSkeleton() { return m_skeleton; }
    void setSkeleton(const Skeleton &skeleton) { m_skeleton = skeleton; }
    void setAnimations(const std::unordered_map<std::string, Animation> &animations) { m_animations = animations; }

    AssetType getType() const override { return AssetType::EModel; }
    std::string getTypeName() const override { return "Model"; }

   private:
    std::vector<Node> m_nodes;
    std::vector<std::shared_ptr<ICE::Mesh>> m_meshes;
    std::vector<ICE::AssetUID> m_materials;
    std::unordered_map<std::string, Animation> m_animations;
    Skeleton m_skeleton;
    AABB m_boundingbox{{0, 0, 0}, {0, 0, 0}};
};
}  // namespace ICE