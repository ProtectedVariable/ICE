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
    // One engine-owned input service, wired to this window's handlers. Pumped once per frame in
    // step() (after the systems have read the frame's input).
    m_input = std::make_unique<InputManager>(m_window);
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

    // Finalize any async imports that completed staging (publish payloads, commit model sub-assets)
    // on the main thread. Done before the early-out so imports drain even with no active scene (e.g.
    // a loading screen). Cheap no-op when nothing is in flight.
    if (project) {
        project->getAssetBank()->pump();
    }

    if (m_active_scene) {
        // Application frame callbacks (engine.onUpdate) run before the ECS systems so gameplay
        // state they set is consumed by animation/scene-graph/render the same frame.
        for (auto& cb : m_update_callbacks) {
            cb(m_delta_time);
        }
        // Extension seams (P11): advance physics and drive the scripting VM before the ECS systems,
        // so their results are visible to animation/scene-graph/render this frame. No-ops when unset.
        if (m_physics) {
            m_physics->step(m_delta_time);
        }
        if (m_scripting) {
            m_scripting->update(m_delta_time);
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

    // Roll input at the END of the frame -- after any onUpdate callbacks and ECS systems (scripts)
    // have read this frame's PRESS/RELEASE edges and mouse position. This is why PRESS lasts exactly
    // one frame and getMouseDelta is the movement across the frame. (Callbacks fire during the run
    // loop's pollEvents, which precedes step(), so rolling here -- not before the systems -- is what
    // keeps the edges alive for the frame that reads them.)
    if (m_input) {
        m_input->update(static_cast<float>(m_delta_time));
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
        existing->setScheduler(m_scheduler);
        if (auto as = registry->tryGetSystem<AnimationSystem>()) {
            as->setScheduler(m_scheduler);
        }
        m_active_scene = scene;
        m_active_render_system = existing;
        return;
    }

    auto renderer = std::make_shared<ForwardRenderer>(api, m_graphics_factory, project->getGPURegistry());
    auto rs = std::make_shared<RenderSystem>(registry, project->getGPURegistry());
    auto as = std::make_shared<AnimationSystem>(registry, project->getAssetBank());
    auto sgs = std::make_shared<SceneGraphSystem>(scene);
    // Scripts get their behaviour context from here: the scene they live in and the engine input
    // service (m_input, constructed in initialize()). Both are non-owning.
    auto ss = std::make_shared<ScriptSystem>(registry, scene.get(), m_input.get());
    rs->setCamera(camera_);
    rs->setRenderer(renderer);
    renderer->setUseRenderGraph(m_use_render_graph);
    rs->setScheduler(m_scheduler);  // null unless parallel systems are enabled
    as->setScheduler(m_scheduler);
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

void ICEEngine::setUseRenderGraph(bool enable) {
    m_use_render_graph = enable;
    // Apply to the active scene's renderer immediately; future scenes pick it up in
    // installRuntimeSystems.
    if (m_active_render_system) {
        if (auto renderer = m_active_render_system->getRenderer()) {
            renderer->setUseRenderGraph(enable);
        }
    }
}

void ICEEngine::setPhysicsBackend(const std::shared_ptr<IPhysicsBackend>& backend) {
    m_physics = backend;
    if (m_physics) {
        m_physics->initialize();
    }
}

void ICEEngine::setScriptingBackend(const std::shared_ptr<IScriptingBackend>& backend) {
    m_scripting = backend;
    if (m_scripting && m_active_scene) {
        m_scripting->initialize(*m_active_scene->getRegistry());
    }
}

void ICEEngine::loadPlugin(const std::shared_ptr<IPlugin>& plugin) {
    if (!plugin) {
        return;
    }
    PluginContext ctx;
    ctx.registry = m_active_scene ? m_active_scene->getRegistry().get() : nullptr;
    ctx.asset_bank = project ? project->getAssetBank().get() : nullptr;
    plugin->registerWith(ctx);
    m_plugins.push_back(plugin);
}

void ICEEngine::enableBackgroundAssetLoading() {
    if (!m_scheduler) {
        m_scheduler = std::make_shared<JobScheduler>();
    }
    if (project) {
        project->getAssetBank()->setScheduler(m_scheduler);
    }
}

void ICEEngine::setParallelSystems(bool enable) {
    if (enable) {
        if (!m_scheduler) {
            m_scheduler = std::make_shared<JobScheduler>();
        }
    } else {
        m_scheduler = nullptr;
    }
    // Share the pool with the asset bank so async imports stage off-thread too (setScheduler drains
    // in-flight loads before dropping the old scheduler). Null returns the bank to inline staging.
    if (project) {
        project->getAssetBank()->setScheduler(m_scheduler);
    }
    // Apply immediately to the active scene's parallel-capable systems; newly activated scenes pick
    // it up in installRuntimeSystems.
    if (m_active_scene) {
        auto registry = m_active_scene->getRegistry();
        if (auto rs = registry->tryGetSystem<RenderSystem>()) {
            rs->setScheduler(m_scheduler);
        }
        if (auto as = registry->tryGetSystem<AnimationSystem>()) {
            as->setScheduler(m_scheduler);
        }
    }
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
