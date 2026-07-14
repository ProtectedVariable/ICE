#include "ICEEngine.h"

#include <EngineConfig.h>
#include <FileUtils.h>
#include <ForwardRenderer.h>
#include <GLFW/glfw3.h>
#include <Logger.h>
#include <OpenGLFactory.h>
#include <PerspectiveCamera.h>
#include <Profiler.h>
#include <TransformComponent.h>
#include <AnimationSystem.h>
#include <ScriptSystem.h>
#include <SceneGraphSystem.h>
#include <WindowFactory.h>

namespace ICE {
ICEEngine::ICEEngine() : camera(std::make_shared<PerspectiveCamera>(60, 16.f / 9.f, 0.1f, 100000)), config(EngineConfig::LoadFromFile()) {
}

ICEEngine::ICEEngine(const Config &cfg) : ICEEngine() {
    // Turn-key path: build the window + graphics backend from cfg, then initialize.
    WindowFactory window_factory;
    auto window = window_factory.createWindow(cfg.windowBackend, cfg.width, cfg.height, cfg.title);
    auto graphics_factory = std::make_shared<OpenGLFactory>();
    initialize(graphics_factory, window);
}

void ICEEngine::initialize(const std::shared_ptr<GraphicsFactory> &graphics_factory, const std::shared_ptr<Window> &window) {
    Logger::Log(Logger::INFO, "Core", "Engine starting up...");
    m_graphics_factory = graphics_factory;
    m_window = window;
    m_window->setSwapInterval(1);
    m_window->setResizeCallback([this](int w, int h) { onFramebufferResize(w, h); });
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
    if (!m_active_scene) {
        return;  // nothing activated yet
    }
    // Application frame callbacks (engine.onUpdate) run before the ECS systems so gameplay
    // state they set is consumed by animation/scene-graph/render the same frame.
    for (auto& cb : m_update_callbacks) {
        cb(m_delta_time);
    }
    // The engine drives the active scene it was handed; it does not reach back through the
    // project/registry to rediscover the render system each frame (it was cached on activation).
    if (m_active_render_system) {
        m_active_render_system->setTarget(m_target_fb);
    }
    {
        ICE_PROFILE_SCOPE("updateSystems");
        m_active_scene->getRegistry()->updateSystems(m_delta_time);
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

void ICEEngine::run() {
    // The application's frame loop, previously hand-written at every call site. Order matches the
    // old inline loop: poll, step (runs onUpdate callbacks + ECS systems), resize the viewport to
    // the current framebuffer, then present.
    while (m_window && !m_window->shouldClose()) {
        m_window->pollEvents();
        step();
        int display_w, display_h;
        m_window->getFramebufferSize(&display_w, &display_h);
        api->setViewport(0, 0, display_w, display_h);
        m_window->swapBuffers();
    }
}

void ICEEngine::installRuntimeSystems(const std::shared_ptr<Scene> &scene, const std::shared_ptr<Camera> &camera_) {
    auto registry = scene->getRegistry();

    // Idempotent by construction: a scene that already has its render system is considered set up.
    // Re-activation just re-points the view camera -- it never builds a second set of systems
    // (which is also why the sample no longer needs, and could not accidentally cause, a duplicate
    // system add).
    if (auto existing = registry->tryGetSystem<RenderSystem>()) {
        existing->setCamera(camera_);
        m_active_scene = scene;
        m_active_render_system = existing;
        return;
    }

    auto renderer = std::make_shared<ForwardRenderer>(api, m_graphics_factory);
    auto rs = std::make_shared<RenderSystem>(registry, project->getGPURegistry());
    auto as = std::make_shared<AnimationSystem>(registry, project->getAssetBank());
    auto sgs = std::make_shared<SceneGraphSystem>(scene);
    auto ss = std::make_shared<ScriptSystem>(registry);
    rs->setCamera(camera_);
    rs->setRenderer(renderer);
    registry->addSystem(rs);
    registry->addSystem(as);
    registry->addSystem(sgs);
    registry->addSystem(ss);
    auto [w, h] = m_window->getSize();
    renderer->resize(w, h);

    // The engine tracks the visible scene and its render system so per-frame/resize code never
    // rediscovers them through project->getCurrentScene()->getRegistry()->getSystem<...>().
    m_active_scene = scene;
    m_active_render_system = rs;
}

void ICEEngine::setupScene(const std::shared_ptr<Camera> &camera_) {
    // Legacy/editor entry point: set up the current scene with the supplied (editor) camera.
    installRuntimeSystems(project->getCurrentScene(), camera_);
    camera = camera_;
}

void ICEEngine::setActiveScene(const std::shared_ptr<Scene> &scene) {
    // The one coupling that makes a scene visible: install its runtime systems (idempotent) and
    // let the engine drive it. The scene owns its camera, so the render system borrows that.
    installRuntimeSystems(scene, scene->cameraPtr());
}

void ICEEngine::onFramebufferResize(int width, int height) {
    // Resize goes straight to the cached active render system -- no reaching back through the
    // project/scene/registry chain.
    if (!m_active_render_system) {
        return;
    }
    m_active_render_system->setViewport(0, 0, width, height);
    if (auto cam = m_active_render_system->getCamera()) {
        cam->resize(width, height);
    }
}

Project &ICEEngine::newProject(const std::string &name, const fs::path &base) {
    auto proj = std::make_shared<Project>(base, name);
    proj->CreateDirectories();
    project = proj;
    // Scenes created on this project become the active scene through us (systems installed once).
    proj->setSceneActivator([this](const std::shared_ptr<Scene> &scene) { setActiveScene(scene); });
    return *proj;
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
