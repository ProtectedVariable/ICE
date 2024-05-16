#pragma once

#include <functional>

namespace ICE {
using MouseEventCallback = std::function<void(float x, float y)>;
class MouseHandler {
 public:
  virtual ~MouseHandler() = default;

  virtual void update() {}
  void addMouseMoveListener(const MouseEventCallback& callback) { m_mousemove_callbacks.push_back(callback); }

 protected:
  std::vector<MouseEventCallback> m_mousemove_callbacks;
};
}  // namespace ICE
