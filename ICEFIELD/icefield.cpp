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
    engine.setProject(std::make_shared<Project>(".", "IceField"));
    engine.getProject()->addScene(Scene("TestScene"));

	engine.getAssetBank()->addAsset<Mesh>("cube", {"cube.obj"});

	engine.getAssetBank()->addAsset<Shader>("solid", {"solid.vs", "solid.fs"});
	auto shader_id = engine.getAssetBank()->getUID("Shaders/solid");

	auto mat = std::make_shared<Material>();
	mat->setShader(shader_id);
	mat->setUniform("uAlbedo", Eigen::Vector3f(0.2, 0.5, 1));
	engine.getAssetBank()->addAsset<Material>("mat", mat);

	auto entity = engine.getProject()->getCurrentScene()->createEntity();
	engine.getProject()->getCurrentScene()->getRegistry().addComponent<TransformComponent>(e, TransformComponent)

	while(!window->shouldClose()) {
		window->pollEvents();

		window->swapBuffers();
	}

	return 0;
}