#include "ICEEngine.h"

#include <EngineConfig.h>
#include <FileUtils.h>
#include <ForwardRenderer.h>
#include <GLFW/glfw3.h>
#include <Logger.h>
#include <PerspectiveCamera.h>
#include <Profiler.h>
#include <TransformComponent.h>
#include <AnimationSystem.h>
#include <SceneGraphSystem.h>

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
    // Seed the frame clock so the first step() doesn't report a huge delta (time since
    // the steady_clock epoch).
    lastFrameTime = std::chrono::steady_clock::now();
}

void ICEEngine::step() {
    // Snapshot the previous frame's profiler samples and start a new frame.
    Profiler::get().beginFrame();
    ICE_PROFILE_SCOPE("Engine::step");

    // Delta time in seconds as a double: the old integer-millisecond cast truncated to 0
    // above ~1000 fps (freezing animation) and lost ~13% at 144 Hz.
    auto now = std::chrono::steady_clock::now();
    m_delta_time = std::chrono::duration<double>(now - lastFrameTime).count();
    lastFrameTime = now;
    if (!project) {
        return;
    }
    auto render_system = project->getCurrentScene()->getRegistry()->getSystem<RenderSystem>();
    render_system->setTarget(m_target_fb);
    {
        ICE_PROFILE_SCOPE("updateSystems");
        project->getCurrentScene()->getRegistry()->updateSystems(m_delta_time);
    }

    // Periodically surface the previous frame's timing breakdown (every ~5s at 60fps).
    static int s_profile_log_counter = 0;
    if (++s_profile_log_counter >= 300) {
        s_profile_log_counter = 0;
        for (const auto& [name, ms] : Profiler::get().lastFrame()) {
            Logger::Log(Logger::DEBUG, "Profiler", "%s: %.3f ms", name.c_str(), ms);
        }
    }
}

void ICEEngine::setupScene(const std::shared_ptr<Camera> &camera_) {
    auto renderer = std::make_shared<ForwardRenderer>(api, m_graphics_factory);
    auto rs = std::make_shared<RenderSystem>(api, m_graphics_factory, project->getCurrentScene()->getRegistry(), project->getGPURegistry());
    auto as = std::make_shared<AnimationSystem>(project->getCurrentScene()->getRegistry(), project->getAssetBank());
    auto sgs = std::make_shared<SceneGraphSystem>(project->getCurrentScene());
    rs->setCamera(camera_);
    rs->setRenderer(renderer);
    project->getCurrentScene()->getRegistry()->addSystem(rs);
    project->getCurrentScene()->getRegistry()->addSystem(as);
    project->getCurrentScene()->getRegistry()->addSystem(sgs);
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
