#pragma once

#include "VertexArray.h"

namespace ICE {
class GPUMesh {
   public:
    explicit GPUMesh(const std::shared_ptr<VertexArray> &vao) {}
    const std::shared_ptr<VertexArray> getVertexArray() const;
};
}  // namespace ICE
