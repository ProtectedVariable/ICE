#include <ICEEngine.h>
#include <WindowFactory.h>
#include <OpenGLFactory.h>

using namespace ICE;

int main(void) {
	ICEEngine engine;
	WindowFactory win_factory;
	auto window = win_factory.createWindow(WindowBackend::GLFW, 1280, 720, "IceField");
	window->makeContextCurrent();
	window->setSwapInterval(1);
	bool err = gl3wInit();
	engine.initialize(std::make_shared<OpenGLFactory>(), window->getHandle());

	while(!window->shouldClose()) {
		window->pollEvents();

		window->swapBuffers();
	}

	return 0;
}