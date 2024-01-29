#include "WindowFactory.h"
#include "GLFWWindow.h"

namespace ICE {
	std::shared_ptr<Window> WindowFactory::createWindow(WindowBackend backend, int width, int height, const std::string &title) {
		if(backend == WindowBackend::GLFW) {
			return std::make_shared<GLFWWindow>(width, height, title);
		}
		return nullptr;
	}
}