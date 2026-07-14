#include <AnimationSystem.h>
#include <ICEEngine.h>
#include <ICEMath.h>
#include <MaterialExporter.h>
#include <NativeScript.h>
#include <NativeScriptComponent.h>
#include <OpenGLFactory.h>
#include <PerspectiveCamera.h>
#include <WindowFactory.h>

using namespace ICE;

// Example per-entity game logic: spin the entity around Y. Attached via a NativeScriptComponent
// (see below); the ScriptSystem instantiates and updates it every frame.
class Rotator : public NativeScript {
   public:
    void onUpdate(double dt) override {
        m_angle += dt * 45.0;  // 45 deg/sec, frame-rate independent
        registry()->getComponent<TransformComponent>(entity())->setRotationEulerDeg({0, (float) m_angle, 0});
    }

   private:
    double m_angle = 0.0;
};

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

    engine.getProject()->copyAssetFile("Models", "Adventurer", "ImportAssets/Adventurer.glb");
    engine.getAssetBank()->addAsset<ICE::Model>("Adventurer", {engine.getProject()->getBaseDirectory() / "Assets" / "Models" / "Adventurer.glb"});

    
    auto sphere = engine.getAssetBank()->getUID(AssetPath::WithTypePrefix<Mesh>("sphere"));
    auto base_mat = engine.getAssetBank()->getUID(AssetPath::WithTypePrefix<Material>("base_mat"));
    for (int i = 0; i < 10; i++) {
        // EntityHandle: create + add components without the registry boilerplate.
        auto entity = scene->create();
        entity.add(TransformComponent({(float) i - 5, 0, 0}, Eigen::Vector3f::Zero(), Eigen::Vector3f::Constant(1.0)));
        entity.add(LightComponent(LightType::PointLight, {1, 1, 1}));
        entity.add(RenderComponent(sphere, base_mat));
    }

    
    auto model_id = engine.getAssetBank()->getUID(AssetPath::WithTypePrefix<Model>("Adventurer"));
    auto entity2 = scene->wrap(scene->spawnTree(model_id, engine.getAssetBank()));
    entity2.add(AnimationComponent{.currentAnimation = "Walk", .loop = true});

    // Attach the Rotator script: the ScriptSystem will spin this entity while it animates.
    NativeScriptComponent rotator;
    rotator.bind<Rotator>();
    entity2.add(rotator);

    auto entity3 = scene->wrap(scene->spawnTree(model_id, engine.getAssetBank()));
    entity3.transform()->setPosition({1, 0, 0});
    entity3.add(AnimationComponent{.currentAnimation = "Run", .loop = true});

    // Global per-frame game logic via the engine.onUpdate hook: switch entity3 to Idle after 5s.
    double elapsed = 0.0;
    bool switched = false;
    engine.onUpdate([&, entity3](double dt) {
        elapsed += dt;
        if (!switched && elapsed > 5.0) {
            switched = true;
            entity3.animation()->playAnimation("Idle", 300.0);
        }
    });
    
    auto camera = std::make_shared<PerspectiveCamera>(60.0, 16.0 / 9.0, 0.01, 10000.0);
    camera->backward(5);
    camera->up(5);
    camera->pitch(-30);
    scene->getRegistry()->getSystem<RenderSystem>()->setCamera(camera);

    while (!window->shouldClose()) {
        window->pollEvents();

        engine.step();

        //Render system duty
        int display_w, display_h;
        window->getFramebufferSize(&display_w, &display_h);
        engine.getApi()->setViewport(0, 0, display_w, display_h);
        window->swapBuffers();
    }

    return 0;
}