#pragma once

#include <memory>
#include <tuple>

#include "KeyboardHandler.h"
#include "MouseHandler.h"

namespace ICE {
using WindowResizeCallback = std::function<void(int, int)>;

class Window {
   public:
    virtual ~Window() {}

    virtual void* getHandle() const = 0;
    virtual bool shouldClose() = 0;
    virtual void close() = 0;
    virtual std::pair<std::shared_ptr<MouseHandler>, std::shared_ptr<KeyboardHandler>> getInputHandlers() const = 0;
    virtual void pollEvents() = 0;
    virtual void swapBuffers() = 0;
    virtual void getFramebufferSize(int* width, int* height) = 0;
    virtual void setSwapInterval(int interval) = 0;
    virtual void makeContextCurrent() = 0;
    virtual void setResizeCallback(const WindowResizeCallback &callback) = 0;
    virtual std::pair<int, int> getSize() const = 0;
};
}  // namespace ICE