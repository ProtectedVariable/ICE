#include <ICEEngine.h>
#include <ICEMath.h>
#include <MaterialExporter.h>
#include <OpenGLFactory.h>
#include <PerspectiveCamera.h>
#include <WindowFactory.h>

using namespace ICE;

int main(void) {
    std::filesystem::remove_all("IceField_project");
    ICEEngine engine;
    WindowFactory win_factory;
    auto window = win_factory.createWindow(WindowBackend::GLFW, 1280, 720, "IceField");
    auto g_factory = std::make_shared<OpenGLFactory>();

    engine.initialize(g_factory, window);

    auto project = std::make_shared<Project>(".", "IceField_project");
    project->CreateDirectories();
    project->addScene(Scene("TestScene"));
    auto scene = project->getScenes().front();
    project->setCurrentScene(scene);

    engine.setProject(project);

    engine.getProject()->copyAssetFile("Models", "glock", "ImportAssets/glock.glb");
    engine.getAssetBank()->addAsset<ICE::Model>("glock", {engine.getProject()->getBaseDirectory() / "Assets" / "Models" / "glock.glb"});

    auto mesh_id = engine.getAssetBank()->getUID(AssetPath::WithTypePrefix<Model>("cube"));

    auto entity = scene->createEntity();
    scene->getRegistry()->addComponent<TransformComponent>(
        entity, TransformComponent(Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero(), Eigen::Vector3f(1, 1, 1)));
    scene->getRegistry()->addComponent<RenderComponent>(entity, RenderComponent(mesh_id));

    auto camera = std::make_shared<PerspectiveCamera>(60.0, 16.0 / 9.0, 0.01, 10000.0);
    camera->backward(2);
    camera->up(1);
    camera->pitch(-30);
    scene->getRegistry()->getSystem<RenderSystem>()->setCamera(camera);

    int i = 0;
    while (!window->shouldClose()) {
        window->pollEvents();

        engine.step();

        scene->getRegistry()->getComponent<TransformComponent>(entity)->rotation().y() = i;

        //Render system duty
        int display_w, display_h;
        window->getFramebufferSize(&display_w, &display_h);
        engine.getApi()->setViewport(0, 0, display_w, display_h);
        window->swapBuffers();
        i++;
    }

    return 0;
}