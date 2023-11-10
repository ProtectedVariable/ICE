#include <ICEEngine.h>
#include <ICEMath.h>
#include <OpenGLFactory.h>
#include <PerspectiveCamera.h>
#include <WindowFactory.h>

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

    auto project = std::make_shared<Project>(".", "IceField");
    project->addScene(Scene("TestScene"));
    auto scene = project->getScenes().front();
    project->setCurrentScene(scene);
    
    engine.setProject(project);

    engine.getAssetBank()->addAsset<Mesh>("cube", {"Assets/cube.obj"});

    engine.getAssetBank()->addAsset<Shader>("solid", {"Assets/solid.vs", "Assets/solid.fs"});
    auto shader_id = engine.getAssetBank()->getUID(std::string("Shaders/solid"));

    auto mat = std::make_shared<Material>();
    mat->setShader(shader_id);
    mat->setUniform("uAlbedo", Eigen::Vector3f(0.2, 0.5, 1));
    engine.getAssetBank()->addAsset<Material>("mat", mat);

    auto mesh_id = engine.getAssetBank()->getUID(std::string("Meshes/cube"));
    auto material_id = engine.getAssetBank()->getUID(std::string("Materials/mat"));

    auto entity = scene->createEntity();
    scene->getRegistry()->addComponent<TransformComponent>(entity, TransformComponent(Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero(), Eigen::Vector3f(1, 1, 1)));
    scene->getRegistry()->addComponent<RenderComponent>(entity, RenderComponent(mesh_id, material_id));

    auto api = g_factory->createRendererAPI();

    auto camera = std::make_shared<PerspectiveCamera>(60.0, 16.0 / 9.0, 0.01, 10000.0);
    camera->backward(2);
    camera->up(1);
    camera->pitch(-30);
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

	scene->getRegistry()->getSystem<RenderSystem>()->setCamera(camera);
    

    int i = 0;
    while (!window->shouldClose()) {
        window->pollEvents();

        engine.step(scene);
    	
		mat->setUniform("uAlbedo", Eigen::Vector3f(abs(sin(i / 100.0)), 0.5, 1));
        scene->getRegistry()->getComponent<TransformComponent>(entity)->rotation.y() = i;

        //Render system duty
        window->swapBuffers();
		i++;
    }

    return 0;
}