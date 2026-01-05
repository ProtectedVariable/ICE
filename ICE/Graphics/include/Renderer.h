//
// Created by Thomas Ibanez on 17.11.20.
//

#pragma once

#include <AssetBank.h>
#include <Entity.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

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
};

struct Skybox {
    std::shared_ptr<Mesh> cube_mesh;
    std::shared_ptr<Shader> shader;
    std::unordered_map<AssetUID, std::shared_ptr<Texture>> textures;
};

struct Drawable {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<Shader> shader;
    std::unordered_map<AssetUID, std::shared_ptr<Texture>> textures;
    Eigen::Matrix4f model_matrix;
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
    virtual void submitSkybox(const Skybox& e) = 0;
    virtual void submitDrawable(const Drawable& e) = 0;
    virtual void submitLight(const Light& e) = 0;
    virtual void prepareFrame(Camera &camera) = 0;
    virtual std::shared_ptr<Framebuffer> render() = 0;
    virtual void endFrame() = 0;
    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual void setClearColor(Eigen::Vector4f clearColor) = 0;
    virtual void setViewport(int x, int y, int w, int h) = 0;
};
}  // namespace ICE
