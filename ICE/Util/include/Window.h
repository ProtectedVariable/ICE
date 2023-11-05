#pragma once

namespace ICE {
class Window {
public:
	virtual ~Window() {}

	virtual void* getHandle() = 0;
	virtual bool shouldClose() = 0;
	virtual void pollEvents() = 0;
	virtual void swapBuffers() = 0;
	virtual void getFramebufferSize(int* width, int* height) = 0;
	virtual void setSwapInterval(int interval) = 0;
	virtual void makeContextCurrent() = 0;
};
}