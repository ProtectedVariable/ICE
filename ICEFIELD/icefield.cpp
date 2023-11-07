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
    engine.setProject(std::make_shared<Project>(".", "IceField"));
    engine.getProject()->addScene(Scene("TestScene"));
    auto scene = engine.getProject()->getScenes().front();
    engine.getProject()->setCurrentScene(scene);

    engine.getAssetBank()->addAsset<Mesh>("cube", {"Assets/cube.obj"});

    engine.getAssetBank()->addAsset<Shader>("solid", {"Assets/solid.vs", "Assets/solid.fs"});
    auto shader_id = engine.getAssetBank()->getUID(std::string("Shaders/solid"));

    auto mat = std::make_shared<Material>();
    mat->setShader(shader_id);
    mat->setUniform("uAlbedo", Eigen::Vector3f(0.2, 0.5, 1));
    engine.getAssetBank()->addAsset<Material>("mat", mat);

    auto entity = scene->createEntity();
    scene->getRegistry()->addComponent<TransformComponent>(entity, TransformComponent(Eigen::Vector3f(), Eigen::Vector3f(), Eigen::Vector3f(1, 1, 1)));

    auto api = g_factory->createRendererAPI();

    auto cube = engine.getAssetBank()->getAsset<Mesh>("cube");
    auto material = engine.getAssetBank()->getAsset<Material>("mat");
    auto shader = engine.getAssetBank()->getAsset<Shader>(material->getShader());

    auto camera = std::make_shared<PerspectiveCamera>(60.0, 16.0 / 9.0, 0.01, 10000.0);
    camera->backward(2);
    camera->up(1);
    camera->pitch(-30);
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    int i = 0;
    while (!window->shouldClose()) {
        window->pollEvents();

        //renderer duty
        api->clear();
        api->bindDefaultFramebuffer();
        api->setViewport(0, 0, 1280, 720);

        shader->bind();

        shader->loadMat4("projection", camera->getProjection());
        shader->loadMat4("view", camera->lookThrough());
        shader->loadMat4("model", model);

        //Per entity
        shader->loadFloat3("uAlbedo", material->getUniform<Eigen::Vector3f>("uAlbedo"));
        api->renderVertexArray(cube->getVertexArray());

        //Render system duty
        window->swapBuffers();

        i++;
        model = Eigen::Matrix4f::Identity() * rotationMatrix({0, (float) i, 0});
    }

    return 0;
}