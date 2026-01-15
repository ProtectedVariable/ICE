#pragma once

#include "VertexArray.h"

namespace ICE {
class GPUMesh {
   public:
    explicit GPUMesh(const std::shared_ptr<VertexArray> &vao) : m_vao(vao) {}
    const std::shared_ptr<VertexArray> getVertexArray() const { return m_vao; }

   private:
    std::shared_ptr<VertexArray> m_vao;
};
}  // namespace ICE
