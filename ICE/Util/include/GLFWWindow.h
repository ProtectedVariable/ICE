#pragma once

#include <GLFW/glfw3.h>

#include <string>

#include "Window.h"

namespace ICE {
class GLFWWindow : public Window {
   public:
    GLFWWindow(int width, int height, const std::string& title);

    void* getHandle() const override;
    bool shouldClose() override;
    void close() override;
    std::pair<std::shared_ptr<MouseHandler>, std::shared_ptr<KeyboardHandler>> getInputHandlers() const override;
    void pollEvents() override;
    void swapBuffers() override;
    void getFramebufferSize(int* width, int* height) override;
    void setSwapInterval(int interval) override;
    void makeContextCurrent() override;
    void setResizeCallback(const WindowResizeCallback& callback) override;
    std::pair<int, int> getSize() const override;

    void windowResized(int w, int h);

   private:
    GLFWwindow* m_handle;
    int m_width;
    int m_height;
    std::shared_ptr<MouseHandler> m_mouse_handler;
    std::shared_ptr<KeyboardHandler> m_keyboard_handler;
    WindowResizeCallback m_resize_callback = [](int, int) {
    };

};
}  // namespace ICE