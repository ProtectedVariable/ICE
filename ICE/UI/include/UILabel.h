#pragma once

#include <Eigen/Dense>
#include <memory>
#include <string>

#include "Font.h"
#include "UIElement.h"

namespace ICE {

// A single line of text, sized to its bounds' height and rendered through the shared Font atlas.
class UILabel : public UIElement {
   public:
    UILabel(const std::string& name, const Eigen::Vector2f& position, const Eigen::Vector2f& size, const std::string& text,
            const Eigen::Vector4f& color, const std::shared_ptr<Font>& font)
        : UIElement(name, position, size),
          m_text(text),
          m_color(color),
          m_font(font) {}

    void render(const UIBound& parent_bounds, const UIRenderContext& ctx) override {
        const UIBound b = computeBounds(parent_bounds);
        if (m_font) {
            // Scale glyphs so the text stands `b.height` pixels tall; baseline near the bottom edge.
            const float scale = b.height / m_font->glyphHeight();
            m_font->renderText(m_text, b.x, b.y + b.height, scale, m_color, ctx);
        }
        for (const auto& child : m_children) {
            child->render(b, ctx);
        }
    }

    void setText(const std::string& text) { m_text = text; }
    void setColor(const Eigen::Vector4f& color) { m_color = color; }

   private:
    std::string m_text;
    Eigen::Vector4f m_color;
    std::shared_ptr<Font> m_font;
};
}  // namespace ICE
