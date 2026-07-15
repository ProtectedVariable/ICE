//
// Created by Thomas Ibanez on 25.11.20.
//

#pragma once

#include <AssetBank.h>
#include <EngineConfig.h>
#include <GL/gl3w.h>
#include <GraphicsAPI.h>
#include <Project.h>
#include <Registry.h>
#include <RenderSystem.h>
#include <System.h>
#include <WindowFactory.h>

#include <functional>
#include <string>
#include <vector>

namespace ICE {
class ICEEngine {
   public:
    // One-call launch configuration for the config constructor below. Aggregate, so it takes
    // designated initializers: ICEEngine engine({ .title = "IceField", .width = 1280 }).
    struct Config {
        std::string title = "ICE";
        int width = 1280;
        int height = 720;
        WindowBackend windowBackend = WindowBackend::GLFW;
        // Renderer backend is OpenGL today; add a selector here when there is more than one.
    };

    ICEEngine();

    // Turn-key construction: create the window and graphics backend from cfg and initialize the
    // engine in one step, so applications don't hand-build a WindowFactory/GraphicsFactory. The
    // default constructor + initialize() path remains for hosts (e.g. the editor) that supply
    // their own window.
    explicit ICEEngine(const Config& cfg);

    void initialize(const std::shared_ptr<GraphicsFactory>& graphics_factor, const std::shared_ptr<Window>& window);

    // Create a project rooted at base/name, prepare it on disk, and adopt it as the current
    // project. Scenes created on the returned project (Project::createScene) are automatically
    // activated by this engine. Returns a reference to the engine-owned project.
    Project& newProject(const std::string& name, const fs::path& base = ".");

    // Make a scene the engine's active (visible) scene: install its runtime systems (render,
    // animation, scene graph, scripting) once and drive it each frame, with its render system
    // pointed at the scene's own camera. Idempotent per scene -- the sole coupling between engine
    // and scene. Called automatically for scenes created via Project::createScene.
    void setActiveScene(const std::shared_ptr<Scene>& scene);

    // The scene the engine is currently driving (nullptr before any activation).
    std::shared_ptr<Scene> getActiveScene() const { return m_active_scene; }

    // Opt into multithreaded systems: creates a shared job scheduler and hands it to the parallel-
    // capable systems (render culling + animation) of the active and future scenes. Off by default
    // -- the single-threaded path is the validated fallback. Passing false tears the scheduler
    // back down and returns those systems to serial.
    void setParallelSystems(bool enable);

    // Opt into driving rendering through the RenderGraph (off by default; the hardcoded pass path
    // is the validated fallback). Applies to the active and future scenes' renderers. Exists to
    // validate the graph path before it becomes the only path.
    void setUseRenderGraph(bool enable);

    void step();

    // Run the blocking main loop until the window closes: poll input, step() the engine, size the
    // viewport to the framebuffer, and present. This owns the boilerplate the application used to
    // write by hand; per-frame game logic goes through onUpdate (called inside step()), so most
    // apps need only build their scene and call run(). Returns when the window requests close.
    void run();

    // Register an application frame callback. Every registered callback is invoked once per
    // step() with the frame delta (seconds), before the ECS systems run -- so gameplay logic
    // here is picked up by animation, the scene graph and rendering the same frame. This is
    // the simplest place to put per-frame game code; for per-entity logic use a NativeScript.
    void onUpdate(const std::function<void(double)>& callback) { m_update_callbacks.push_back(callback); }

    // Duration of the last step() in seconds.
    double getDeltaTime() const { return m_delta_time; }

    // Legacy/editor activation: set up the project's current scene with an externally-owned
    // (editor) camera. Retained for the editor; the clean path is newProject + createScene, which
    // route through setActiveScene with the scene's own camera.
    void setupScene(const std::shared_ptr<Camera>& camera_);

    // The editor's viewport camera (distinct from a scene's own view camera, which Scene owns).
    // Persisted via Project::writeToFile. Kept until the editor migrates to the scene camera.
    std::shared_ptr<Camera> getCamera();

    std::shared_ptr<AssetBank> getAssetBank();
    std::shared_ptr<GPURegistry> getGPURegistry();

    Entity getSelected() const;

    std::shared_ptr<RendererAPI> getApi() const;

    std::shared_ptr<Project> getProject() const;

    void setProject(const std::shared_ptr<Project>& project);

    void setSelected(Entity selected);

    EngineConfig& getConfig();

    std::shared_ptr<GraphicsFactory> getGraphicsFactory() const;

    std::shared_ptr<Context> getContext() const;

    std::shared_ptr<Framebuffer> getInternalFramebuffer() const;
    void setRenderFramebufferInternal(bool use_internal);

    std::shared_ptr<Window> getWindow() const;

   private:
    // Shared system-building used by both setupScene (legacy/editor path) and setActiveScene:
    // builds the render/animation/scene-graph/script systems on the scene's registry with the
    // given view camera, and caches the scene + its render system as the active pair. Idempotent.
    void installRuntimeSystems(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Camera>& camera);

    // Window framebuffer-resize handler: resizes the active render system's viewport and camera.
    void onFramebufferResize(int width, int height);

    std::shared_ptr<GraphicsFactory> m_graphics_factory;
    std::shared_ptr<Context> ctx;
    std::shared_ptr<RendererAPI> api;
    std::shared_ptr<Framebuffer> internalFB;
    std::shared_ptr<Framebuffer> m_target_fb = nullptr;
    std::shared_ptr<Window> m_window;

    std::shared_ptr<Camera> camera;  // editor viewport camera (see getCamera)
    std::shared_ptr<Project> project = nullptr;

    // The scene the engine currently drives and its render system, cached on activation so the
    // per-frame and resize paths don't rediscover them through the project/registry each time.
    std::shared_ptr<Scene> m_active_scene;
    std::shared_ptr<RenderSystem> m_active_render_system;

    // Shared job scheduler for the parallel-capable systems; null unless setParallelSystems(true).
    std::shared_ptr<JobScheduler> m_scheduler;

    // Whether renderers should drive the frame through the RenderGraph (see setUseRenderGraph).
    bool m_use_render_graph = false;

    std::chrono::steady_clock::time_point lastFrameTime;
    double m_delta_time = 0.0;

    std::vector<std::function<void(double)>> m_update_callbacks;

    EngineConfig config;
};
}  // namespace ICE
