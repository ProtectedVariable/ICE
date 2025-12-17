//
// Created by Thomas Ibanez on 16.11.20.
//

#pragma once

#include <Eigen/Dense>

#include "Component.h"
#include "ICEMath.h"

namespace ICE {
struct TransformComponent : public Component {
    TransformComponent(const Eigen::Vector3f &pos, const Eigen::Vector3f &rot, const Eigen::Vector3f &sca)
        : m_position(pos),
          m_rotation(rot),
          m_scale(sca) {}

    Eigen::Vector3f getPosition() const { return m_position; }
    Eigen::Vector3f getRotation() const { return m_rotation; }
    Eigen::Vector3f getScale() const { return m_scale; }

    Eigen::Matrix4f getModelMatrix() const {
        if (m_dirty) {
            m_model_matrix = transformationMatrix(m_position, m_rotation, m_scale);
            m_dirty = false;
        }
        return m_model_matrix;
    }

    Eigen::Vector3f &position() {
        m_dirty = true;
        return m_position;
    }
    Eigen::Vector3f &rotation() {
        m_dirty = true;
        return m_rotation;
    }
    Eigen::Vector3f &scale() {
        m_dirty = true;
        return m_scale;
    }

    void setPosition(const Eigen::Vector3f &_position) {
        m_dirty = true;
        m_position = _position;
    }
    void setRotation(const Eigen::Vector3f &_rotation) {
        m_dirty = true;
        m_rotation = _rotation;
    }
    void setScale(const Eigen::Vector3f &_scale) {
        m_dirty = true;
        m_scale = _scale;
    }

   private:
    Eigen::Vector3f m_position, m_rotation, m_scale;
    mutable Eigen::Matrix4f m_model_matrix;
    mutable bool m_dirty = true;
};
}  // namespace ICE
