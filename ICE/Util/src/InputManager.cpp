#include "InputManager.h"

namespace ICE {
InputManager::InputManager(const std::shared_ptr<ICE::Window> &window) : m_window(window) {

    auto [mouse_handler, keyboard_handler] = window->getInputHandlers();

    keyboard_handler->addKeyPressedListener([this](ICE::Key key) {
        m_key_states[key] = KeyState::DOWN;
        m_key_actions[key] = KeyAction::PRESS;
    });
    keyboard_handler->addKeyReleaseListener([this](ICE::Key key) {
        m_key_states[key] = KeyState::UP;
        m_key_actions[key] = KeyAction::RELEASE;
    });
    mouse_handler->addMouseClickListener([this](ICE::MouseButton button, ICE::ButtonAction action) {
        m_mouse_states[button] = action == ICE::ButtonAction::PRESS ? KeyState::DOWN : KeyState::UP;
        m_mouse_actions[button] = action == ICE::ButtonAction::PRESS ? KeyAction::PRESS : KeyAction::RELEASE;
    });
    mouse_handler->addMouseMoveListener([this](float x, float y) {
        if (!m_has_mouse) {
            // First cursor position: seed the "previous" position too, so the first getMouseDelta is
            // zero instead of a jump from the origin.
            m_prev_x = x;
            m_prev_y = y;
            m_has_mouse = true;
        }
        m_x = x;
        m_y = y;
    });
}

void InputManager::update(float dt) {
    for (const auto &[k, v] : m_key_actions) {
        m_key_actions[k] = KeyAction::NONE;
    }
    for (const auto &[k, v] : m_mouse_actions) {
        m_mouse_actions[k] = KeyAction::NONE;
    }
    m_prev_x = m_x;
    m_prev_y = m_y;
}

KeyAction InputManager::getKeyAction(ICE::Key key) {
    if (m_key_actions.contains(key)) {
        return m_key_actions[key];
    }
    return KeyAction::NONE;
}

KeyState InputManager::getKeyState(ICE::Key key) {
    if (m_key_states.contains(key)) {
        return m_key_states[key];
    }
    return KeyState::UP;
}

bool InputManager::isKeyDown(ICE::Key key) {
    return getKeyState(key) == KeyState::DOWN;
}

KeyAction InputManager::getMouseAction(ICE::MouseButton button) {
    if (m_mouse_actions.contains(button)) {
        return m_mouse_actions[button];
    }
    return KeyAction::NONE;
}

KeyState InputManager::getMouseState(ICE::MouseButton button) {
    if (m_mouse_states.contains(button)) {
        return m_mouse_states[button];
    }
    return KeyState::UP;
}

float InputManager::getMouseX() {
    return m_x;
}

float InputManager::getMouseY() {
    return m_y;
}

float InputManager::getMouseAngle() {
    auto [ww, wh] = m_window->getSize();
    return std::atan2(m_x - ww/2.0, m_y - wh/2.0);
}

Eigen::Vector2f InputManager::getMouseDelta() {
    return {m_x - m_prev_x, m_y - m_prev_y};
}
}  // namespace ICE