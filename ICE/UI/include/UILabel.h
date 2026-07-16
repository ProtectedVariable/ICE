#pragma once

#include <GraphicsFactory.h>

#include "Font.h"
#include "GraphicsUtil.h"
#include "UIElement.h"

namespace ICE {
class UILabel : public UIElement {
 public:
  UILabel(const std::string &name, const Eigen::Vector2f &position, const Eigen::Vector2f &size,
          const std::string &text, const Eigen::Vector4f &color, const std::shared_ptr<Font> &font)
      : UIElement(name, position, size),
        m_text(text),
        m_color(color),
        m_font(font) {}

  void render(const UIBound &absolute_bounds, const Eigen::Matrix4f &projection,
              const std::shared_ptr<RendererAPI> &api) override {
    Eigen::Vector2f abs_size =
        m_relative_size.cwiseProduct(Eigen::Vector2f{absolute_bounds.width, absolute_bounds.height});

    Eigen::Vector2f abs_pos(0, 0);
    if (m_relative_pos.x() >= 0) {
      abs_pos.x() = m_relative_pos.x() * absolute_bounds.width + absolute_bounds.x;
    } else {
      abs_pos.x() = (1 + m_relative_pos.x()) * absolute_bounds.width + absolute_bounds.x - abs_size.x();
    }

    if (m_relative_pos.y() >= 0) {
      abs_pos.y() = m_relative_pos.y() * absolute_bounds.height + absolute_bounds.y;
    } else {
      abs_pos.y() = (1 + m_relative_pos.y()) * absolute_bounds.height + absolute_bounds.y - abs_size.y();
    }

    m_font->renderText(m_text, abs_pos.x(), abs_pos.y() + abs_size.y(), abs_size.y() / m_font->horizontal_norm(),
                       m_color, projection, api);

    for (const auto &child : m_children) {
      child->render({abs_pos.x(), abs_pos.y(), abs_size.x(), abs_size.y()}, projection, api);
    }
  }

  void update() override {}

  void setText(const std::string &text) { m_text = text; }
  void setColor(const Eigen::Vector4f &color) { m_color = color; }

 private:
  std::string m_text;
  Eigen::Vector4f m_color;
  std::shared_ptr<Font> m_font;
};
}  // namespace ICE