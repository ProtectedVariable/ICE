#include <AnimationSystem.h>
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

    engine.getApi()->setClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    engine.setProject(project);
    project->getCurrentScene()->getRegistry()->addSystem(std::make_shared<AnimationSystem>(scene->getRegistry(), engine.getAssetBank()));

    engine.getProject()->copyAssetFile("Models", "glock", "ImportAssets/glock.glb");
    engine.getAssetBank()->addAsset<ICE::Model>("glock", {engine.getProject()->getBaseDirectory() / "Assets" / "Models" / "glock.glb"});
    engine.getProject()->copyAssetFile("Models", "pistol", "ImportAssets/pistol.glb");
    engine.getAssetBank()->addAsset<ICE::Model>("pistol", {engine.getProject()->getBaseDirectory() / "Assets" / "Models" / "pistol.glb"});
    engine.getProject()->copyAssetFile("Models", "Adventurer", "ImportAssets/Adventurer.glb");
    engine.getAssetBank()->addAsset<ICE::Model>("Adventurer", {engine.getProject()->getBaseDirectory() / "Assets" / "Models" / "Adventurer.glb"});

    auto entity = scene->createEntity();
    scene->getRegistry()->addComponent<TransformComponent>(entity,
                                                           TransformComponent({0, 100, 0}, Eigen::Vector3f::Zero(), Eigen::Vector3f(0.1, 0.1, 0.1)));
    scene->getRegistry()->addComponent<LightComponent>(entity, LightComponent(LightType::PointLight, {1, 1, 1}));

    auto model_id = engine.getAssetBank()->getUID(AssetPath::WithTypePrefix<Model>("Adventurer"));
    auto entity2 = scene->spawnTree(model_id, engine.getAssetBank());
    scene->getRegistry()->addComponent<AnimationComponent>(entity2, AnimationComponent{.currentAnimation = "Walk", .loop = true});

    auto entity3 = scene->spawnTree(model_id, engine.getAssetBank());
    scene->getRegistry()->getComponent<TransformComponent>(entity3)->setPosition({1, 0, 0});
    scene->getRegistry()->addComponent<AnimationComponent>(entity3, AnimationComponent{.currentAnimation = "Run", .loop = true});

    auto camera = std::make_shared<PerspectiveCamera>(60.0, 16.0 / 9.0, 0.01, 10000.0);
    camera->backward(5);
    camera->up(5);
    camera->pitch(-30);
    scene->getRegistry()->getSystem<RenderSystem>()->setCamera(camera);

    int i = 0;
    while (!window->shouldClose()) {
        window->pollEvents();

        engine.step();

        scene->getRegistry()->getComponent<TransformComponent>(entity2)->setRotationEulerDeg({0, i / 10.0f, 0});

        //Render system duty
        int display_w, display_h;
        window->getFramebufferSize(&display_w, &display_h);
        engine.getApi()->setViewport(0, 0, display_w, display_h);
        window->swapBuffers();
        i++;
    }

    return 0;
}