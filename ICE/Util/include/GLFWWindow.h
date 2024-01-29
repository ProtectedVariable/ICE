#include "Window.h"
#include <GLFW/glfw3.h>
#include <string>

namespace ICE {
class GLFWWindow : public Window {
public:

	GLFWWindow(int width, int height, const std::string &title);

	void* getHandle() override;
	bool shouldClose() override;
	void pollEvents() override;
	void swapBuffers() override;
	void getFramebufferSize(int* width, int* height) override;
	void setSwapInterval(int interval) override;
	void makeContextCurrent() override;

private:
	GLFWwindow* m_handle;
};
}