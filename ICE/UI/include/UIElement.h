#pragma once

#include <GraphicsAPI.h>

#include <Eigen/Dense>
#include <string>

namespace ICE {

enum class EventType { Click, MouseEnter, MouseLeave };

struct Event {
  EventType type;
  int button;
};

struct UIBound {
  float x;
  float y;
  float width;
  float height;
};

class UIElement {

 public:
  UIElement(const std::string &name, const Eigen::Vector2f &position, const Eigen::Vector2f &size)
      : m_name(name),
        m_relative_pos(position),
        m_relative_size(size) {}
  virtual ~UIElement() = default;

  virtual void render(const UIBound &absolute_bounds, const Eigen::Matrix4f &projection,
                      const std::shared_ptr<ICE::RendererAPI> &api) = 0;
  virtual void update() = 0;

  void addChild(std::unique_ptr<UIElement> &&child) {
    child->m_parent = this;
    m_children.push_back(std::move(child));
  }

  std::string getName() const { return m_name; }
  bool isVisible() const { return m_visible; }
  const std::vector<std::unique_ptr<UIElement>> &getChildren() const { return m_children; }

  void setPosition(const Eigen::Vector2f &position) { m_relative_pos = position; }
  void setSize(const Eigen::Vector2f &size) { m_relative_size = size; }
  void setVisible(bool visible) { m_visible = visible; }

 protected:
  std::string m_name;
  Eigen::Vector2f m_relative_pos;
  Eigen::Vector2f m_relative_size;
  bool m_visible = true;
  UIElement *m_parent = nullptr;
  std::vector<std::unique_ptr<UIElement>> m_children;
};
}  // namespace ICE
