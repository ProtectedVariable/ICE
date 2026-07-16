#pragma once

#include <Window.h>

#include <Eigen/Dense>
#include <memory>

namespace ICE {
enum class KeyState { UP, DOWN };
enum class KeyAction { NONE, PRESS, RELEASE };

class InputManager {
 public:
  InputManager(const std::shared_ptr<ICE::Window> &window);
  void update(float dt);
  KeyAction getKeyAction(ICE::Key key);
  KeyState getKeyState(ICE::Key key);
  bool isKeyDown(ICE::Key key);

  KeyAction getMouseAction(ICE::MouseButton button);
  KeyState getMouseState(ICE::MouseButton button);

  float getMouseX();
  float getMouseY();

  // Cursor movement since the previous update() (frame-to-frame). Zero until the cursor is first
  // seen, so there is no spurious jump on the first frame / after (re)gaining focus.
  Eigen::Vector2f getMouseDelta();

 private:
  std::shared_ptr<ICE::Window> m_window;

  std::unordered_map<ICE::Key, KeyState> m_key_states;
  std::unordered_map<ICE::Key, KeyAction> m_key_actions;
  std::unordered_map<ICE::MouseButton, KeyState> m_mouse_states;
  std::unordered_map<ICE::MouseButton, KeyAction> m_mouse_actions;

  float m_x = 0.0f;
  float m_y = 0.0f;
  float m_prev_x = 0.0f;
  float m_prev_y = 0.0f;
  bool m_has_mouse = false;  // false until the first cursor position arrives
};
}  // namespace ICE