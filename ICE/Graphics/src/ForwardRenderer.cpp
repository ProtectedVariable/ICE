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
        skybox_cmd.is_instanced = false;
        m_render_commands.push_back(skybox_cmd);
    }

    // Instance batching: Group drawables by mesh/material/shader
    std::unordered_map<uint64_t, std::vector<const Drawable*>> instance_batches;
    std::vector<const Drawable*> non_instanced_drawables;  // Skinned meshes, etc.
    
    for (const auto& drawable : m_drawables) {
        // Skip instancing for skinned meshes (has bones)
        if (!drawable.bone_matrices.empty()) {
            non_instanced_drawables.push_back(&drawable);
            continue;
        }
        
        // Create batch key (mesh + material + shader)
        uint64_t batch_key = 0;
        batch_key ^= reinterpret_cast<uint64_t>(drawable.mesh.get()) + 0x9e3779b9 + (batch_key << 6) + (batch_key >> 2);
        batch_key ^= reinterpret_cast<uint64_t>(drawable.material.get()) + 0x9e3779b9 + (batch_key << 6) + (batch_key >> 2);
        batch_key ^= reinterpret_cast<uint64_t>(drawable.shader.get()) + 0x9e3779b9 + (batch_key << 6) + (batch_key >> 2);
        
        instance_batches[batch_key].push_back(&drawable);
    }
    
    // Convert batches to render commands
    m_instance_batches.clear();  // Clear previous frame's instance data
    
    for (const auto& [key, batch] : instance_batches) {
        if (batch.size() == 1) {
            // Single instance - use regular rendering
            const auto* drawable = batch[0];
            RenderCommand cmd;
            cmd.mesh = drawable->mesh;
            cmd.material = drawable->material;
            cmd.shader = drawable->shader;
            cmd.textures = drawable->textures;
            cmd.model_matrix = drawable->model_matrix;
            cmd.depthTest = true;
            cmd.faceCulling = true;
            cmd.is_instanced = false;
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
            RenderCommand cmd;
            cmd.mesh = first->mesh;
            cmd.material = first->material;
            cmd.shader = first->shader;
            cmd.textures = first->textures;
            cmd.depthTest = true;
            cmd.faceCulling = true;
            cmd.is_instanced = true;
            cmd.instance_count = batch.size();
            cmd.instance_data = &instance_data_vec;  // Link to stored data
            m_render_commands.push_back(cmd);
        }
    }
    
    // Add non-instanced drawables (skinned meshes)
    for (const auto* drawable : non_instanced_drawables) {
        RenderCommand cmd;
        cmd.mesh = drawable->mesh;
        cmd.material = drawable->material;
        cmd.shader = drawable->shader;
        cmd.textures = drawable->textures;
        cmd.model_matrix = drawable->model_matrix;
        cmd.depthTest = true;
        cmd.faceCulling = true;
        cmd.bones = drawable->bone_matrices;
        cmd.is_instanced = false;
        m_render_commands.push_back(cmd);
    }

    // Improved sorting: opaque/transparent, then shader, then material, then depth
    Eigen::Vector3f camera_pos = camera.getPosition();
    std::sort(m_render_commands.begin(), m_render_commands.end(), 
        [&camera_pos](const RenderCommand& a, const RenderCommand& b) {
            bool a_transparent = a.material ? a.material->isTransparent() : false;
            bool b_transparent = b.material ? b.material->isTransparent() : false;
            
            // 1. Opaque before transparent
            if (a_transparent != b_transparent) {
                return !a_transparent;  // opaque (false) < transparent (true)
            }
            
            // 2. Sort by shader (minimize shader switches)
            if (a.shader.get() != b.shader.get()) {
                return a.shader.get() < b.shader.get();
            }
            
            // 3. Sort by material (minimize material switches)
            if (a.material.get() != b.material.get()) {
                return a.material.get() < b.material.get();
            }
            
            // 4. Sort by depth
            if (!a_transparent) {
                // Front-to-back for opaque (early-z optimization)
                float dist_a = (a.model_matrix.block<3,1>(0,3) - camera_pos).squaredNorm();
                float dist_b = (b.model_matrix.block<3,1>(0,3) - camera_pos).squaredNorm();
                return dist_a < dist_b;
            } else {
                // Back-to-front for transparent (correct blending)
                float dist_a = (a.model_matrix.block<3,1>(0,3) - camera_pos).squaredNorm();
                float dist_b = (b.model_matrix.block<3,1>(0,3) - camera_pos).squaredNorm();
                return dist_a > dist_b;
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
