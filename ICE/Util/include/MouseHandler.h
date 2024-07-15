#pragma once

#include <functional>

namespace ICE {

enum class MouseButton { LEFT_MOUSE_BUTTON, RIGHT_MOUSE_BUTTON, MIDDLE_MOUSE_BUTTON, OTHER_MOUSE_BUTTON };
enum class ButtonAction { PRESS, RELEASE };

using MouseMoveEventCallback = std::function<void(float x, float y)>;
using MouseButtonEventCallback = std::function<void(MouseButton, ButtonAction)>;
class MouseHandler {
   public:
    virtual ~MouseHandler() = default;

    virtual void update() {}
    virtual void setGrabMouse(bool grab) = 0;
    void addMouseMoveListener(const MouseMoveEventCallback& callback) { m_mousemove_callbacks.push_back(callback); }
    void addMouseClickListener(const MouseButtonEventCallback& callback) { m_mousebutton_callbacks.push_back(callback); }

   protected:
    std::vector<MouseMoveEventCallback> m_mousemove_callbacks;
    std::vector<MouseButtonEventCallback> m_mousebutton_callbacks;
};
}  // namespace ICE
