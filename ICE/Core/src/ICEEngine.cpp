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
#include <AssetPath.h>
#include <AudioSystem.h>
#include <NullAudioBackend.h>
#include <OpenALAudioFactory.h>
#include <ScriptSystem.h>
#include <SceneGraphSystem.h>
#include <Shader.h>
#include <UIManager.h>
#include <UIRenderPass.h>
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
    // Silence audio while the window is in the background. Uses setSuspended rather than
    // setMuted so that regaining focus cannot undo a mute the user (or the editor) set.
    m_window->setFocusCallback([this](bool focused) {
        if (m_audio) {
            m_audio->setSuspended(!focused);
        }
    });
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

        // Audio housekeeping runs after the systems so it observes this frame's final state
        // (positions the scene graph just resolved, voices gameplay just started). Mixing itself
        // happens on the backend's own thread; this only reclaims finished voices.
        if (m_audio) {
            ICE_PROFILE_SCOPE("audio");
            m_audio->update(m_delta_time);
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

    // Hit-test the UI against this frame's pointer state, before the input roll below consumes the
    // click edge. Bridges the input service to the (input-agnostic) UIManager.
    if (m_ui && m_input && m_window) {
        auto [w, h] = m_window->getSize();
        const bool clicked = m_input->getMouseAction(MouseButton::LEFT_MOUSE_BUTTON) == KeyAction::PRESS;
        m_ui->processInput(m_input->getMouseX(), m_input->getMouseY(), clicked, w, h);
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
        // Re-activation re-points the ear as well as the eye: the scene's active camera entity may
        // have changed since this scene was last activated.
        if (auto aus = registry->tryGetSystem<AudioSystem>()) {
            aus->setListenerEntity(scene->getActiveCamera());
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
    rs->setScheduler(m_scheduler);  // null unless parallel systems are enabled
    as->setScheduler(m_scheduler);
    registry->addSystem(rs);
    registry->addSystem(as);
    registry->addSystem(sgs);
    registry->addSystem(ss);

    // Positional audio. audio() lazily builds the audio service (and its backend) on first use, so
    // a scene only pays for it if something asks. The listener defaults to the scene's active
    // camera entity -- hear from where you see, unless an AudioListenerComponent says otherwise.
    if (auto* audio_engine = audio()) {
        auto aus = std::make_shared<AudioSystem>(registry, audio_engine);
        aus->setListenerEntity(scene->getActiveCamera());
        registry->addSystem(aus);
    }
    auto [w, h] = m_window->getSize();
    renderer->resize(w, h);

    // The engine tracks the visible scene and its render system so per-frame/resize code never
    // rediscovers them through project->getCurrentScene()->getRegistry()->getSystem<...>().
    m_active_scene = scene;
    m_active_render_system = rs;

    // A fresh renderer was just built, so the UI pass (if any) is not on it yet.
    m_ui_pass_registered = false;
    registerUIPass();
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

void ICEEngine::setAudioBackend(const std::shared_ptr<IAudioBackend>& backend) {
    // Tear the old service down first: AudioEngine holds an AudioRegistry whose buffers belong to
    // the outgoing backend and must be released while it is still alive.
    m_audio.reset();
    if (m_audio_backend) {
        m_audio_backend->shutdown();
    }

    m_audio_backend = backend;
    if (!m_audio_backend) {
        return;
    }
    if (!m_audio_backend->initialize(AudioDeviceConfig{})) {
        Logger::Log(Logger::WARNING, "Audio", "Audio backend failed to initialize; falling back to the null backend.");
        m_audio_backend = std::make_shared<NullAudioBackend>();
        m_audio_backend->initialize(AudioDeviceConfig{});
    }
    if (project) {
        m_audio = std::make_unique<AudioEngine>(m_audio_backend, project->getAssetBank());
    }
}

AudioEngine* ICEEngine::audio() {
    if (m_audio) {
        return m_audio.get();
    }
    // Clips are resolved through the project's asset bank, so there is nothing to attach to yet.
    if (!project) {
        return nullptr;
    }
    if (!m_audio_backend) {
        // Default backend, chosen here so nothing in core names a concrete audio API beyond this
        // one line. A machine with no audio device is normal (CI, a server build), so a failed
        // open degrades to silence rather than being treated as an error.
        m_audio_backend = OpenALAudioFactory{}.createBackend();
        if (!m_audio_backend->initialize(AudioDeviceConfig{})) {
            m_audio_backend = std::make_shared<NullAudioBackend>();
            m_audio_backend->initialize(AudioDeviceConfig{});
        }
    }
    // If a scheduler already exists, streaming decode goes on it rather than inline.
    m_audio_backend->setScheduler(m_scheduler);
    m_audio = std::make_unique<AudioEngine>(m_audio_backend, project->getAssetBank());
    return m_audio.get();
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
    // Streaming audio decodes on the same pool; without it a music refill decodes inline and
    // spikes whichever frame needs it.
    if (m_audio_backend) {
        m_audio_backend->setScheduler(m_scheduler);
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
    if (m_audio_backend) {
        m_audio_backend->setScheduler(m_scheduler);
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

std::shared_ptr<Renderer> ICEEngine::renderer() const {
    // Reaches through the cached active render system rather than the
    // project/scene/registry/getSystem chain that application code used to write by hand.
    return m_active_render_system ? m_active_render_system->getRenderer() : nullptr;
}

UIManager *ICEEngine::ui() {
    if (!m_ui) {
        if (!project || !m_graphics_factory) {
            return nullptr;  // no project yet: no "ui" shader asset and no font to load
        }
        auto shader = project->getGPURegistry()->getShader(AssetPath::WithTypePrefix<Shader>("ui"));
        const auto font_path = (project->getBaseDirectory() / "Assets" / "Fonts" / "helvetica.ttf").string();
        if (!shader) {
            // Loud, not silent: without the shader the UI draws nothing. Usually means the updated
            // Assets/ (ui.shader.json + glsl/ui.*) weren't deployed next to the executable.
            Logger::Log(Logger::ERROR, "UI", "'ui' shader asset failed to load -- UI will not render (check Assets/Shaders/ui.*)");
        }
        m_ui = std::make_shared<UIManager>(m_graphics_factory, shader, font_path);
    }
    registerUIPass();
    if (!m_ui_pass_registered) {
        Logger::Log(Logger::WARNING, "UI", "ui() called before a scene/renderer is active -- the UI pass is not attached yet");
    }
    return m_ui.get();
}

void ICEEngine::registerUIPass() {
    if (!m_ui || m_ui_pass_registered || !m_active_render_system) {
        return;
    }
    if (auto renderer = m_active_render_system->getRenderer()) {
        renderer->addPass(std::make_unique<UIRenderPass>(m_ui.get()));
        m_ui_pass_registered = true;
    }
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
    // A new project brings its own asset bank, so any audio service built against the previous
    // one is stale. Drop it; audio() rebuilds it (keeping the initialized backend) on next use.
    m_audio.reset();
    setupScene(camera);
    applyProjectMixer();
}

void ICEEngine::applyProjectMixer() {
    if (!project) {
        return;
    }
    auto* audio_engine = audio();
    if (audio_engine == nullptr) {
        return;
    }
    // Empty means the project never authored a mix; leave the engine at its defaults.
    const auto& gains = project->getBusGains();
    for (std::size_t i = 0; i < gains.size() && i < static_cast<std::size_t>(BusId::Count); ++i) {
        audio_engine->setBusGain(static_cast<BusId>(i), gains[i]);
    }
    const auto& mutes = project->getBusMutes();
    for (std::size_t i = 0; i < mutes.size() && i < static_cast<std::size_t>(BusId::Count); ++i) {
        audio_engine->setBusMuted(static_cast<BusId>(i), mutes[i]);
    }
}

void ICEEngine::storeProjectMixer() {
    if (!project || !m_audio) {
        return;
    }
    const auto count = static_cast<std::size_t>(BusId::Count);
    std::vector<float> gains(count);
    std::vector<bool> mutes(count);
    for (std::size_t i = 0; i < count; ++i) {
        gains[i] = m_audio->getBusGain(static_cast<BusId>(i));
        mutes[i] = m_audio->isBusMuted(static_cast<BusId>(i));
    }
    project->setBusGains(gains);
    project->setBusMutes(mutes);
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
