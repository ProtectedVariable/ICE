//
// Created by Thomas Ibanez on 20.11.20.
//

#include "ForwardRenderer.h"

#include <GL/gl3w.h>
#include <ICEMath.h>
#include <LightComponent.h>
#include <Logger.h>
#include <RenderComponent.h>
#include <Scene.h>
#include <TransformComponent.h>

#include <unordered_set>

namespace ICE {

ForwardRenderer::ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory)
    : m_api(api),
      m_geometry_pass(api, factory, {1, 1, 1}) {

    m_camera_ubo = factory->createUniformBuffer(sizeof(CameraUBO), 0);
    m_light_ubo = factory->createUniformBuffer(sizeof(SceneLightsUBO), 1);
}

void ForwardRenderer::submitSkybox(const Skybox& e) {
    m_skybox.emplace(e);
}
void ForwardRenderer::submitDrawable(const Drawable& e) {
    m_drawables.push_back(e);
}
void ForwardRenderer::submitLight(const Light& e) {
    m_lights.push_back(e);
}

void ForwardRenderer::prepareFrame(Camera& camera) {
    //TODO: Sort entities, make shader list, batch, make instances, set uniforms, etc..

    auto view_mat = camera.lookThrough();
    auto proj_mat = camera.getProjection();

    CameraUBO camera_ubo_data{.projection = proj_mat, .view = view_mat};
    m_camera_ubo->putData(&camera_ubo_data, sizeof(CameraUBO));

    SceneLightsUBO light_ubo_data;
    light_ubo_data.light_count = m_lights.size();
    light_ubo_data.ambient_light = Eigen::Vector4f(0.1f, 0.1f, 0.1f, 1.0f);
    for (int i = 0; i < m_lights.size(); i++) {
        auto light = m_lights[i];
        light_ubo_data.lights[i].position = light.position;
        light_ubo_data.lights[i].rotation = light.rotation;
        light_ubo_data.lights[i].color = light.color;
        light_ubo_data.lights[i].distance_dropoff = light.distance_dropoff;
        light_ubo_data.lights[i].type = static_cast<int>(light.type);
    }
    m_light_ubo->putData(&light_ubo_data, sizeof(SceneLightsUBO));

    if (m_skybox.has_value()) {
        RenderCommand skybox_cmd;
        skybox_cmd.mesh = m_skybox->cube_mesh;
        skybox_cmd.material = nullptr;
        skybox_cmd.shader = m_skybox->shader;
        skybox_cmd.textures = m_skybox->textures;
        skybox_cmd.model_matrix = Eigen::Matrix4f::Identity();
        m_render_commands.push_back(skybox_cmd);
    }

    for (const auto& drawable : m_drawables) {
        RenderCommand cmd;
        cmd.mesh = drawable.mesh;
        cmd.material = drawable.material;
        cmd.shader = drawable.shader;
        cmd.textures = drawable.textures;
        cmd.model_matrix = drawable.model_matrix;
        cmd.depthTest = true;
        cmd.faceCulling = true;
        cmd.bones = drawable.bone_matrices;
        m_render_commands.push_back(cmd);
    }

    std::sort(m_render_commands.begin(), m_render_commands.end(), [this](const RenderCommand& a, const RenderCommand& b) {
        bool a_transparent = a.material ? a.material->isTransparent() : false;
        bool b_transparent = b.material ? b.material->isTransparent() : false;

        if (!a_transparent && b_transparent) {
            return true;
        } else {
            return false;
        }
    });

    m_geometry_pass.submit(&m_render_commands);
}

std::shared_ptr<Framebuffer> ForwardRenderer::render() {
    m_geometry_pass.execute();
    auto result = m_geometry_pass.getResult();
    return result;
}

void ForwardRenderer::endFrame() {
    m_skybox.reset();
    m_drawables.clear();
    m_lights.clear();
    m_api->checkAndLogErrors();
    m_render_commands.clear();
}

void ForwardRenderer::resize(uint32_t width, uint32_t height) {
    m_api->setViewport(0, 0, width, height);
    m_geometry_pass.resize(width, height);
}

void ForwardRenderer::setClearColor(Eigen::Vector4f clearColor) {
    m_api->setClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
}

void ForwardRenderer::setViewport(int x, int y, int w, int h) {
    m_api->setViewport(x, y, w, h);
}

}  // namespace ICE
