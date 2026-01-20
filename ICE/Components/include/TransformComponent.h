#pragma once

#include <Eigen/Dense>

#include "Component.h"
#include "ICEMath.h"

namespace ICE {

struct TransformComponent : public Component {
    TransformComponent(const Eigen::Vector3f& pos = Eigen::Vector3f::Zero(), const Eigen::Quaternionf& rot = Eigen::Quaternionf::Identity(),
                       const Eigen::Vector3f& sca = Eigen::Vector3f::Ones())
        : m_position(pos),
          m_rotation(rot),
          m_scale(sca) {}

    TransformComponent(const Eigen::Vector3f& pos = Eigen::Vector3f::Zero(), const Eigen::Vector3f& rot = Eigen::Vector3f::Zero(),
                       const Eigen::Vector3f& sca = Eigen::Vector3f::Ones())
        : m_position(pos),
          m_scale(sca) {
        setRotationEulerDeg(rot);
    }

   TransformComponent(const Eigen::Matrix4f& matrix) {
        m_position = matrix.block<3, 1>(0, 3);

        Eigen::Vector3f vX = matrix.block<3, 1>(0, 0);
        Eigen::Vector3f vY = matrix.block<3, 1>(0, 1);
        Eigen::Vector3f vZ = matrix.block<3, 1>(0, 2);

        m_scale << vX.norm(), vY.norm(), vZ.norm();

        if (matrix.block<3, 3>(0, 0).determinant() < 0) {
            m_scale.x() *= -1.0f;
            vX *= -1.0f;  // Flip the axis for rotation calculation
        }

        vX.normalize();
        vY.normalize();
        vZ.normalize();

        Eigen::Matrix3f rot_matrix;
        rot_matrix << vX, vY, vZ;

        m_rotation = Eigen::Quaternionf(rot_matrix);
    }

    Eigen::Vector3f getPosition() const { return m_position; }
    Eigen::Quaternionf getRotation() const { return m_rotation; }
    Eigen::Vector3f getScale() const { return m_scale; }

    Eigen::Vector3f getRotationEulerDeg() const { return m_rotation.toRotationMatrix().eulerAngles(0, 1, 2) * 180.0 / M_PI; }

    Eigen::Matrix4f getModelMatrix() const {
        if (m_dirty) {
            m_model_matrix = Eigen::Matrix4f::Identity();

            // Translation
            m_model_matrix.block<3, 1>(0, 3) = m_position;

            // Rotation
            m_model_matrix.block<3, 3>(0, 0) = m_rotation.toRotationMatrix();

            // Scale
            m_model_matrix.block<3, 3>(0, 0) *= m_scale.asDiagonal();

            m_dirty = false;
        }
        return m_model_matrix;
    }

    Eigen::Matrix4f getWorldMatrix() const {
        if (m_dirty) {
            m_world_matrix = m_parent_matrix * getModelMatrix();
        }
        return m_world_matrix;
    }

    void updateParentMatrix(const Eigen::Matrix4f& parent_matrix) {
        m_parent_matrix = parent_matrix;
        m_world_matrix = parent_matrix * getModelMatrix();
    }

    Eigen::Vector3f& position() {
        m_dirty = true;
        return m_position;
    }

    Eigen::Quaternionf& rotation() {
        m_dirty = true;
        return m_rotation;
    }

    Eigen::Vector3f& scale() {
        m_dirty = true;
        return m_scale;
    }

    void setPosition(const Eigen::Vector3f& position) {
        m_dirty = true;
        m_position = position;
    }

    void setRotation(const Eigen::Quaternionf& rotation) {
        m_dirty = true;
        m_rotation = rotation.normalized();
    }

    void setRotationEulerDeg(const Eigen::Vector3f& eulerDeg) {
        m_dirty = true;
        Eigen::Vector3f rad = eulerDeg * M_PI / 180.0;
        m_rotation = Eigen::AngleAxisf(rad.x(), Eigen::Vector3f::UnitX()) * Eigen::AngleAxisf(rad.y(), Eigen::Vector3f::UnitY())
            * Eigen::AngleAxisf(rad.z(), Eigen::Vector3f::UnitZ());
    }

    void setScale(const Eigen::Vector3f& scale) {
        m_dirty = true;
        m_scale = scale;
    }

   private:
    Eigen::Vector3f m_position;
    Eigen::Quaternionf m_rotation;
    Eigen::Vector3f m_scale;

    mutable Eigen::Matrix4f m_model_matrix = Eigen::Matrix4f::Identity();
    mutable Eigen::Matrix4f m_parent_matrix = Eigen::Matrix4f::Identity();
    mutable Eigen::Matrix4f m_world_matrix = Eigen::Matrix4f::Identity();
    mutable bool m_dirty = true;
};

}  // namespace ICE
