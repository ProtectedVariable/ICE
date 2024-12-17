#define STB_IMAGE_IMPLEMENTATION
#include "ICEEngine.h"

#include <EngineConfig.h>
#include <FileUtils.h>
#include <ForwardRenderer.h>
#include <GLFW/glfw3.h>
#include <Logger.h>
#include <PerspectiveCamera.h>
#include <TransformComponent.h>

namespace ICE {
ICEEngine::ICEEngine() : camera(std::make_shared<PerspectiveCamera>(60, 16.f / 9.f, 0.1f, 100000)), config(EngineConfig::LoadFromFile()) {
}

void ICEEngine::initialize(const std::shared_ptr<GraphicsFactory> &graphics_factory, const std::shared_ptr<Window> &window) {
    Logger::Log(Logger::INFO, "Core", "Engine starting up...");
    m_graphics_factory = graphics_factory;
    m_window = window;
    m_window->setSwapInterval(1);
    m_window->setResizeCallback([this](int w, int h) {
        if (project) {
            project->getCurrentScene()->getRegistry()->getSystem<RenderSystem>()->setViewport(0, 0, w, h);
            project->getCurrentScene()->getRegistry()->getSystem<RenderSystem>()->getCamera()->resize(w, h);
        }
    });
    ctx = graphics_factory->createContext(m_window);
    ctx->initialize();
    api = graphics_factory->createRendererAPI();
    api->initialize();
    internalFB = graphics_factory->createFramebuffer({720, 720, 1});
}

void ICEEngine::step() {
    auto render_system = project->getCurrentScene()->getRegistry()->getSystem<RenderSystem>();
    render_system->setTarget(m_target_fb);
    project->getCurrentScene()->getRegistry()->updateSystems(0.0);
}

void ICEEngine::setupScene() {
    auto renderer = std::make_shared<ForwardRenderer>(api, m_graphics_factory, project->getCurrentScene()->getRegistry(), project->getAssetBank());
    auto rs = std::make_shared<RenderSystem>();
    rs->setCamera(camera);
    rs->setRenderer(renderer);
    project->getCurrentScene()->getRegistry()->addSystem(rs);

    auto [w, h] = m_window->getSize();
    renderer->resize(w, h);
}

std::shared_ptr<Camera> ICEEngine::getCamera() {
    return camera;
}

std::shared_ptr<AssetBank> ICEEngine::getAssetBank() {
    return project->getAssetBank();
}

Eigen::Vector4i ICEEngine::getPickingTextureAt(int x, int y) {
    /* pickingFB->bind();
    pickingFB->resize(gui.getSceneViewportWidth(), gui.getSceneViewportHeight());
    api->setViewport(0, 0, gui.getSceneViewportWidth(), gui.getSceneViewportHeight());
    camera.setParameters({60, (float) gui.getSceneViewportWidth() / (float) gui.getSceneViewportHeight(), 0.01f, 1000});
    api->setClearColor(0, 0, 0, 0);
    api->clear();
    getAssetBank()->getAsset<Shader>("__ice__picking_shader")->bind();
    getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("projection", camera.getProjection());
    getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("view", camera.lookThrough());
    int id = 1;
    for (auto e : currentScene->getRegistry()->getEntities()) {
        //getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("model", e->getComponent<TransformComponent>()->getTransformation());
        //getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadInt("objectID", id++);
        //if(e->hasComponent<RenderComponent>()) {
        //    api->renderVertexArray(getAssetBank()->getAsset<Mesh>(e->getComponent<RenderComponent>()->getMesh())->getVertexArray());
        //}
    }
    auto color = internalFB->readPixel(x, y);
    internalFB->unbind();
    return color;
    */
    return Eigen::Vector4i();
}

std::shared_ptr<RendererAPI> ICEEngine::getApi() const {
    return api;
}

std::shared_ptr<Project> ICEEngine::getProject() const {
    return project;
}

std::shared_ptr<Framebuffer> ICEEngine::getInternalFramebuffer() const {
    return internalFB;
}

void ICEEngine::setRenderFramebufferInternal(bool use_internal) {
    if (use_internal) {
        m_target_fb = internalFB;
    } else {
        m_target_fb = nullptr;
    }
}

void ICEEngine::setProject(const std::shared_ptr<Project> &project) {
    this->project = project;
    this->camera->getPosition() = project->getCameraPosition();
    this->camera->getRotation() = project->getCameraRotation();
    setupScene();
}

EngineConfig &ICEEngine::getConfig() {
    return config;
}

std::shared_ptr<GraphicsFactory> ICEEngine::getGraphicsFactory() const {
    return m_graphics_factory;
}

std::shared_ptr<Context> ICEEngine::getContext() const {
    return ctx;
}

int import_cnt = 0;
void ICEEngine::importMesh() {
    const std::string file = FileUtils::openFileDialog("obj");
    if (file != "") {
        std::string aname = "imported_mesh_" + std::to_string(import_cnt++);
        getAssetBank()->addAsset<Mesh>(aname, {file});
        project->copyAssetFile("Meshes", aname, file);
    }
}

void ICEEngine::importTexture(bool cubeMap) {
    const std::string file = FileUtils::openFileDialog("");
    if (file != "") {
        std::string aname = "imported_texture_" + std::to_string(import_cnt++);
        if (cubeMap) {
            getAssetBank()->addAsset<TextureCube>(aname, {file});
        } else {
            getAssetBank()->addAsset<Texture2D>(aname, {file});
        }
        project->copyAssetFile("Textures", aname, file);
    }
}
}  // namespace ICE
