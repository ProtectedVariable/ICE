#pragma once

#include <GPUTexture.h>

#include <Eigen/Dense>
#include <memory>
#include <string>

#include "ICEMath.h"
#include "UIElement.h"

namespace ICE {

// A solid or textured rectangle. Owns no GPU resources: it draws with the shader and quad handed in
// via the UIRenderContext (see UIElement.h), which is what removed the old per-class static shader.
class UIRect : public UIElement {
   public:
    UIRect(const std::string& name, const Eigen::Vector2f& position, const Eigen::Vector2f& size, const Eigen::Vector4f& color)
        : UIElement(name, position, size),
          m_color(color) {}

    void render(const UIBound& parent_bounds, const UIRenderContext& ctx) override {
        const UIBound b = computeBounds(parent_bounds);
        const Eigen::Matrix4f model = transformationMatrix({b.x, b.y, 0}, {0, 0, 0}, {b.width, b.height, 0});

        ctx.shader->bind();
        ctx.shader->loadInt("uMode", m_texture ? 1 : 0);  // 1 = textured, 0 = solid colour
        ctx.shader->loadFloat4("uColor", m_color);
        ctx.shader->loadFloat2("uUVOffset", {0.0f, 0.0f});
        ctx.shader->loadFloat2("uUVScale", {1.0f, 1.0f});
        ctx.shader->loadMat4("model", model);
        ctx.shader->loadMat4("projection", ctx.projection);
        if (m_texture) {
            m_texture->bind(0);
            ctx.shader->loadInt("uTexture", 0);
        }

        ctx.quad->bind();
        ctx.quad->getIndexBuffer()->bind();
        ctx.api->renderVertexArray(ctx.quad);

        for (const auto& child : m_children) {
            child->render(b, ctx);
        }
    }

    void setTexture(const std::shared_ptr<GPUTexture>& texture) { m_texture = texture; }
    void setColor(const Eigen::Vector4f& color) { m_color = color; }

   private:
    Eigen::Vector4f m_color;
    std::shared_ptr<GPUTexture> m_texture;
};
}  // namespace ICE
