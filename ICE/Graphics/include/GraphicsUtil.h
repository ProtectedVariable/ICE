#pragma once

#include <memory>
#include <vector>

#include "GraphicsFactory.h"
#include "VertexArray.h"

namespace ICE {
namespace GraphicsUtil {

// A unit quad spanning [0,1] x [0,1], indexed as two triangles, with a single 2-component vertex
// attribute at location 0. The positions double as UVs (0..1), so a caller scales the quad to its
// pixel size with a model matrix and samples a texture/glyph with fUV = position. Used by the UI
// elements (UIRect, UILabel, Font).
inline std::shared_ptr<VertexArray> getNormalizedQuad(const std::shared_ptr<GraphicsFactory>& factory) {
    static const std::vector<float> positions = {
        0.0f, 0.0f,  // bottom-left
        1.0f, 0.0f,  // bottom-right
        0.0f, 1.0f,  // top-left
        1.0f, 1.0f,  // top-right
    };
    static const std::vector<int> indices = {0, 1, 2, 2, 1, 3};

    auto vao = factory->createVertexArray();
    auto vbo = factory->createVertexBuffer();
    vbo->putData(positions.data(), positions.size() * sizeof(float));
    vao->pushVertexBuffer(vbo, 2);  // 2-component position, location 0
    auto ibo = factory->createIndexBuffer();
    ibo->putData(indices.data(), indices.size() * sizeof(int));
    vao->setIndexBuffer(ibo);
    return vao;
}

}  // namespace GraphicsUtil
}  // namespace ICE
