#include <ICEEngine.h>
#include <WindowFactory.h>
#include <OpenGLFactory.h>

using namespace ICE;

int main(void) {
	ICEEngine engine;
	WindowFactory win_factory;
	auto window = win_factory.createWindow(WindowBackend::GLFW, 1280, 720, "IceField");
	auto g_factory = std::make_shared<OpenGLFactory>();
	auto context = g_factory->createContext(window);
	context->initialize();
	window->setSwapInterval(1);
	engine.initialize(g_factory, window);
    engine.setProject(std::make_shared<ICE::Project>(".", "IceField"));
    engine.getProject()->addScene(ICE::Scene("TestScene"));



	while(!window->shouldClose()) {
		window->pollEvents();

		window->swapBuffers();
	}

	return 0;
}