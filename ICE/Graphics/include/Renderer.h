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

#include "Camera.h"
#include "Context.h"
#include "Framebuffer.h"
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

class Renderer {
   public:
    virtual ~Renderer() = default;
    virtual void submitSkybox(const Skybox &e) = 0;
    // By value so the caller's temporary (with its texture/bone maps) can be moved into the
    // renderer's queue instead of copied.
    virtual void submitDrawable(Drawable e) = 0;
    virtual void submitLight(const Light &e) = 0;
    virtual void prepareFrame(Camera &camera) = 0;
    virtual std::shared_ptr<Framebuffer> render() = 0;
    virtual void endFrame() = 0;
    // Composite the last render() result onto `target` (nullptr = the default framebuffer) using
    // the given full-screen present shader. The final blit lives in the renderer so that the
    // visible-set producer (RenderSystem) issues no GL of its own.
    virtual void present(const std::shared_ptr<Framebuffer> &target, const std::shared_ptr<ShaderProgram> &present_shader) = 0;
    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual void setClearColor(Eigen::Vector4f clearColor) = 0;
    virtual void setViewport(int x, int y, int w, int h) = 0;

    // Opt into driving the frame through the RenderGraph instead of the hardcoded pass sequence.
    // Off by default; the two paths render identically, so this exists to validate the graph path
    // (and, later, to add post/shadow passes without touching System/Scene). No-op for backends
    // that don't implement a graph.
    virtual void setUseRenderGraph(bool) {}
};
}  // namespace ICE
