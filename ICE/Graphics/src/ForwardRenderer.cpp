//
// Created by Thomas Ibanez on 20.11.20.
//

#include "ForwardRenderer.h"

#include <GL/gl3w.h>
#include <ICEMath.h>

#include <algorithm>
#include <LightComponent.h>
#include <Logger.h>
#include <Profiler.h>
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
void ForwardRenderer::submitDrawable(Drawable e) {
    m_drawables.push_back(std::move(e));
}
void ForwardRenderer::submitLight(const Light& e) {
    m_lights.push_back(e);
}

void ForwardRenderer::prepareFrame(Camera& camera) {
    auto view_mat = camera.lookThrough();
    auto proj_mat = camera.getProjection();
    auto cam_pos = camera.getPosition();

    CameraUBO camera_ubo_data{
        .projection = proj_mat, .view = view_mat, .cameraPos = Eigen::Vector4f(cam_pos.x(), cam_pos.y(), cam_pos.z(), 1.0f)};
    m_camera_ubo->putData(&camera_ubo_data, sizeof(CameraUBO));

    SceneLightsUBO light_ubo_data;
    // Clamp to the UBO's fixed capacity: lights[] is MAX_LIGHTS long, so writing more
    // would overflow the stack-allocated struct.
    const size_t light_count = std::min(m_lights.size(), static_cast<size_t>(MAX_LIGHTS));
    light_ubo_data.light_count = static_cast<int>(light_count);
    light_ubo_data.ambient_light = Eigen::Vector4f(0.1f, 0.1f, 0.1f, 1.0f);
    for (size_t i = 0; i < light_count; i++) {
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
        skybox_cmd.mesh = m_skybox->cube_mesh.get();
        skybox_cmd.material = nullptr;
        skybox_cmd.shader = m_skybox->shader.get();
        skybox_cmd.textures = &m_skybox->textures;
        skybox_cmd.model_matrix = Eigen::Matrix4f::Identity();
        skybox_cmd.is_instanced = false;
        // Draw after opaque geometry (so it only fills background pixels) but before
        // transparent. Its fragments sit at the far plane (z=w in skybox.vs), so it needs
        // GL_LEQUAL and must not write depth.
        skybox_cmd.depthTest = true;
        skybox_cmd.depthWrite = false;
        skybox_cmd.depth_func = DepthFunc::LEqual;
        skybox_cmd.sort_key = 0x7FFFFFFFFFFFFFFFULL;  // last among opaque (transparent bit 63 = 0)
        m_render_commands.push_back(skybox_cmd);
    }

    // Instance batching: group drawables by the exact (mesh, material, shader) triple.
    std::map<BatchKey, std::vector<const Drawable*>> instance_batches;
    std::vector<const Drawable*> non_instanced_drawables;  // Skinned meshes, etc.

    for (const auto& drawable : m_drawables) {
        // Skip instancing for skinned meshes (has bones)
        if (!drawable.bone_matrices.empty()) {
            non_instanced_drawables.push_back(&drawable);
            continue;
        }

        BatchKey key{drawable.mesh.get(), drawable.material.get(), drawable.shader.get()};
        instance_batches[key].push_back(&drawable);
    }
    
    // Convert batches to render commands
    m_instance_batches.clear();  // Clear previous frame's instance data
    Eigen::Vector3f camera_pos = camera.getPosition();

    for (const auto& [key, batch] : instance_batches) {
        if (batch.size() == 1) {
            // Single instance - use regular rendering
            const auto* drawable = batch[0];
            auto dist = (drawable->model_matrix.block<3, 1>(0, 3) - camera_pos).squaredNorm();
            RenderCommand cmd;
            cmd.mesh = drawable->mesh.get();
            cmd.material = drawable->material.get();
            cmd.shader = drawable->shader.get();
            cmd.textures = &drawable->textures;
            cmd.model_matrix = drawable->model_matrix;
            cmd.depthTest = true;
            cmd.faceCulling = true;
            cmd.is_instanced = false;
            cmd.blend = cmd.material->isTransparent();
            cmd.computeSortKey(cmd.material->isTransparent(), dist);
            m_render_commands.push_back(cmd);
        } else {
            // Multiple instances - use instanced rendering
            // Store instance data in member variable for lifetime management
            auto& instance_data_vec = m_instance_batches[key];
            instance_data_vec.clear();
            instance_data_vec.reserve(batch.size());
            
            for (const auto* drawable : batch) {
                InstanceData inst_data;
                inst_data.model_matrix = drawable->model_matrix;
                instance_data_vec.push_back(inst_data);
            }
            
            const auto* first = batch[0];
            auto dist = (first->model_matrix.block<3, 1>(0, 3) - camera_pos).squaredNorm();
            RenderCommand cmd;
            cmd.mesh = first->mesh.get();
            cmd.material = first->material.get();
            cmd.shader = first->shader.get();
            cmd.textures = &first->textures;
            cmd.depthTest = true;
            cmd.faceCulling = true;
            cmd.is_instanced = true;
            cmd.instance_count = batch.size();
            cmd.instance_data = &instance_data_vec;  // Link to stored data
            cmd.blend = cmd.material->isTransparent();
            cmd.computeSortKey(cmd.material->isTransparent(), dist);
            m_render_commands.push_back(cmd);
        }
    }
    
    // Add non-instanced drawables (skinned meshes)
    for (const auto* drawable : non_instanced_drawables) {
        RenderCommand cmd;
        auto dist = (drawable->model_matrix.block<3, 1>(0, 3) - camera_pos).squaredNorm();
        cmd.mesh = drawable->mesh.get();
        cmd.material = drawable->material.get();
        cmd.shader = drawable->shader.get();
        cmd.textures = &drawable->textures;
        cmd.model_matrix = drawable->model_matrix;
        cmd.depthTest = true;
        cmd.faceCulling = true;
        cmd.bones = &drawable->bone_matrices;
        cmd.is_instanced = false;
        cmd.blend = cmd.material->isTransparent();
        cmd.computeSortKey(cmd.material->isTransparent(), dist);
        m_render_commands.push_back(cmd);
    }

    std::sort(m_render_commands.begin(), m_render_commands.end());

    m_geometry_pass.submit(&m_render_commands);
}

std::shared_ptr<Framebuffer> ForwardRenderer::render() {
    m_api->beginGPUTimer();
    m_geometry_pass.execute();
    Profiler::get().addSample("GPU::geometry", m_api->endGPUTimer());
    return m_geometry_pass.getResult();
}

void ForwardRenderer::endFrame() {
    m_skybox.reset();
    m_drawables.clear();
    m_lights.clear();
#ifndef NDEBUG
    // Only drain GL errors in debug builds; skip the per-frame glGetError round-trip in release.
    m_api->checkAndLogErrors();
#endif
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
