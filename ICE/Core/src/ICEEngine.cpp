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
    auto now = std::chrono::steady_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameTime).count();
    lastFrameTime = now;
    auto render_system = project->getCurrentScene()->getRegistry()->getSystem<RenderSystem>();
    render_system->setTarget(m_target_fb);
    project->getCurrentScene()->getRegistry()->updateSystems(dt);
}

void ICEEngine::setupScene(const std::shared_ptr<Camera> &camera_) {
    auto renderer = std::make_shared<ForwardRenderer>(api, m_graphics_factory);
    auto rs = std::make_shared<RenderSystem>(api, m_graphics_factory, project->getCurrentScene()->getRegistry(), project->getGPURegistry());
    rs->setCamera(camera_);
    rs->setRenderer(renderer);
    project->getCurrentScene()->getRegistry()->addSystem(rs);
    camera = camera_;
    auto [w, h] = m_window->getSize();
    renderer->resize(w, h);
}

std::shared_ptr<Camera> ICEEngine::getCamera() {
    return camera;
}

std::shared_ptr<AssetBank> ICEEngine::getAssetBank() {
    return project->getAssetBank();
}

std::shared_ptr<GPURegistry> ICEEngine::getGPURegistry() {
    return project->getGPURegistry();
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

std::shared_ptr<Window> ICEEngine::getWindow() const {
    return m_window;
}

void ICEEngine::setProject(const std::shared_ptr<Project> &project) {
    this->project = project;
    this->camera->getPosition() = project->getCameraPosition();
    this->camera->getRotation() = project->getCameraRotation();
    setupScene(camera);
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
}  // namespace ICE
