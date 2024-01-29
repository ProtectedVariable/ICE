#pragma once

#include <memory>
#include <string>

#include "Window.h"

namespace ICE {

enum class WindowBackend { GLFW };

class WindowFactory {
   public:
    std::shared_ptr<Window> createWindow(WindowBackend backend, int width, int height, const std::string &title);
};
}  // namespace ICE