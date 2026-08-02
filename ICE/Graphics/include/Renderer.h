//
// Created by Thomas Ibanez on 17.11.20.
//

#pragma once

#include <AssetBank.h>
#include <Entity.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <GPUMesh.h>
#include <GpuHandle.h>
#include <LightComponent.h>

#include <memory>
#include <optional>
#include <vector>

#include "Camera.h"
#include "Context.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "RenderFeature.h"
#include "RendererConfig.h"

namespace ICE {
class Scene;

constexpr int MAX_LIGHTS = 16;

struct alignas(16) LightUBO {
    Eigen::Vector3f position;
    float __padding0;  //align to vec4
    Eigen::Vector3f rotation;
    float __padding1;  //align to vec4
    Eigen::Vector3f color;

    float distance_dropoff;
    int type;
};

struct alignas(16) SceneLightsUBO {
    LightUBO lights[MAX_LIGHTS];
    Eigen::Vector4f ambient_light;
    int light_count;
    float __padding[3];
};

struct alignas(16) CameraUBO {
    Eigen::Matrix4f projection;
    Eigen::Matrix4f view;
    Eigen::Vector4f cameraPos;  // world-space camera position (xyz); matches std140 SceneData
};

// The per-frame submission structs carry generational GPU handles, not shared_ptr<GPU*>: no
// refcount churn, no per-drawable texture-map copy. The renderer resolves the mesh/shader handles
// to raw pointers once (in prepareFrame), and the geometry pass resolves each material's texture
// handles at bind. Material is a CPU asset, so it stays a shared_ptr.
struct Skybox {
    MeshHandle cube_mesh;
    ShaderHandle shader;
};

struct Drawable {
    MeshHandle mesh;
    std::shared_ptr<Material> material;
    ShaderHandle shader;
    // The material's asset UID (Material doesn't store its own id -- it's the bank key). Carried so
    // the renderer can build a stable sort key without hashing pointer addresses.
    AssetUID material_uid = NO_ASSET_ID;
    Eigen::Matrix4f model_matrix;
    std::unordered_map<int, Eigen::Matrix4f> bone_matrices;
};

struct Light {
    Eigen::Vector3f position;
    Eigen::Vector3f rotation;
    Eigen::Vector3f color;
    float distance_dropoff;
    LightType type;
};

// Everything the renderer needs to draw one frame, produced by the RenderSystem and handed over in a
// single drawFrame() call. The system produces the visible set; the renderer owns the frame's
// structure (prepare + graph + present).
struct FrameInputs {
    Camera *camera = nullptr;                   // the frame's view
    std::vector<Drawable> drawables;            // the culled, visible geometry
    std::vector<Light> lights;
    std::optional<Skybox> skybox;
    std::shared_ptr<Framebuffer> outputTarget;  // present destination; null = window default framebuffer
    std::shared_ptr<ShaderProgram> presentShader;
};

class Renderer {
   public:
    virtual ~Renderer() = default;
    virtual void submitSkybox(const Skybox &e) = 0;
    // By value so the caller's temporary (with its texture/bone maps) can be moved into the
    // renderer's queue instead of copied.
    virtual void submitDrawable(Drawable e) = 0;
    virtual void submitLight(const Light &e) = 0;
    virtual void prepareFrame(Camera &camera) = 0;
    // Render the frame end to end through the render graph -- geometry, application passes/features,
    // and the final present composite are all graph passes. Nothing is returned: what reaches the
    // screen (or an off-screen target) is decided entirely by the pipeline's present pass.
    virtual void render() = 0;
    virtual void endFrame() = 0;

    // Draw one frame from a gathered input bundle: the single call the RenderSystem makes. The
    // default orchestrates the frame through the primitives above (configure present, submit the
    // visible set, prepare, render, end), so a backend gets it for free; a backend may override to
    // consume the inputs more directly.
    virtual void drawFrame(FrameInputs inputs) {
        setPresentTarget(inputs.outputTarget);
        setPresentShader(inputs.presentShader);
        if (inputs.skybox) {
            submitSkybox(*inputs.skybox);
        }
        for (auto &drawable : inputs.drawables) {
            submitDrawable(std::move(drawable));
        }
        for (const auto &light : inputs.lights) {
            submitLight(light);
        }
        prepareFrame(*inputs.camera);
        render();
        endFrame();
    }

    // Where and how the frame's present pass composites the scene colour. Target nullptr = the
    // window's default framebuffer; a framebuffer = off-screen (the editor's render-to-texture
    // viewport). Set per frame before render(); the present pass reads them at execute time, so
    // changing the target never forces a graph recompile.
    virtual void setPresentTarget(const std::shared_ptr<Framebuffer> &target) {}
    virtual void setPresentShader(const std::shared_ptr<ShaderProgram> &present_shader) {}

    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual void setClearColor(Eigen::Vector4f clearColor) = 0;
    virtual void setViewport(int x, int y, int w, int h) = 0;

    // Register a single render pass: it is added to the frame's render graph (setup() when the
    // graph is built, execute() when it runs), so application code can extend the frame with no
    // engine edits. This is the common path -- prefer it over wrapping one pass in a feature. The
    // renderer takes ownership (the graph is rebuilt from scratch each compile, so it cannot own
    // passes itself). No-op for backends without a graph.
    virtual void addPass(std::unique_ptr<IRenderPass>) {}

    // Register a bundle of passes that belong together (shared state, one on/off switch). For a
    // lone pass use addPass(). Same ownership as addPass.
    virtual void addFeature(std::unique_ptr<RenderFeature>) {}

    // Replace the pipeline that assembles the frame's graph. The engine installs a default
    // (ForwardPipeline); an application swaps in its own to define a custom frame -- deferred
    // shading, post-process chains, a different pass order -- since no pass is privileged. The graph
    // is rebuilt with the new pipeline on the next frame. No-op for backends without a graph.
    virtual void setPipeline(std::unique_ptr<Pipeline>) {}
};
}  // namespace ICE
