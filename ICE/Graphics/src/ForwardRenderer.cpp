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

#include "RenderData.h"

namespace ICE {

ForwardRenderer::ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory,
                                 const std::shared_ptr<Registry>& registry, const std::shared_ptr<AssetBank>& assetBank)
    : m_api(api),
      m_registry(registry),
      m_asset_bank(assetBank),
      m_geometry_pass(api, factory, {1, 1, 1}) {

    m_quad_vao = factory->createVertexArray();
    auto quad_vertex_vbo = factory->createVertexBuffer();
    quad_vertex_vbo->putData(full_quad_v.data(), full_quad_v.size() * sizeof(float));
    m_quad_vao->pushVertexBuffer(quad_vertex_vbo, 3);
    auto quad_uv_vbo = factory->createVertexBuffer();
    quad_uv_vbo->putData(full_quad_tx.data(), full_quad_tx.size() * sizeof(float));
    m_quad_vao->pushVertexBuffer(quad_uv_vbo, 2);
    auto quad_ibo = factory->createIndexBuffer();
    quad_ibo->putData(full_quad_idx.data(), full_quad_idx.size() * sizeof(int));
    m_quad_vao->setIndexBuffer(quad_ibo);

    m_camera_ubo = factory->createUniformBuffer(sizeof(CameraUBO), 0);
    m_light_ubo = factory->createUniformBuffer(sizeof(SceneLightsUBO), 1);
}

void ForwardRenderer::submit(Entity e) {
    remove(e);
    if (m_registry->entityHasComponent<RenderComponent>(e)) {
        m_render_queue.emplace_back(e);
    }
    if (m_registry->entityHasComponent<LightComponent>(e)) {
        m_lights.emplace_back(e);
    }
    if (m_registry->entityHasComponent<SkyboxComponent>(e)) {
        m_skybox = e;
    }
}

void ForwardRenderer::remove(Entity e) {
    auto queue_pos = std::ranges::find(m_render_queue, e);
    if (queue_pos != m_render_queue.end()) {
        m_render_queue.erase(queue_pos);
    }
    auto light_pos = std::ranges::find(m_lights, e);
    if (light_pos != m_lights.end()) {
        m_lights.erase(light_pos);
    }
    if (e == m_skybox) {
        m_skybox = NO_ASSET_ID;
    }
}

void ForwardRenderer::prepareFrame(Camera& camera) {
    //TODO: Sort entities, make shader list, batch, make instances, set uniforms, etc..

    if (m_skybox != NO_ASSET_ID) {
        auto shader = m_asset_bank->getAsset<Shader>("__ice_skybox_shader");
        shader->bind();
        shader->loadMat4("projection", camera.getProjection());
        Eigen::Matrix4f view = camera.lookThrough();
        view(0, 3) = 0;
        view(1, 3) = 0;
        view(2, 3) = 0;
        shader->loadMat4("view", view);
        shader->loadInt("skybox", 0);

        auto skybox = m_registry->getComponent<SkyboxComponent>(m_skybox);
        auto mesh = m_asset_bank->getAsset<Model>("cube")->getMeshes().at(0);
        auto tex = m_asset_bank->getAsset<TextureCube>(skybox->texture);
        m_render_commands.push_back(RenderCommand{.mesh = mesh,
                                                  .material = nullptr,
                                                  .shader = shader,
                                                  .textures = {{skybox->texture, tex}},
                                                  .faceCulling = false,
                                                  .depthTest = false});
    }

    auto view_mat = camera.lookThrough();
    auto proj_mat = camera.getProjection();

    CameraUBO camera_ubo_data{.projection = proj_mat, .view = view_mat};
    m_camera_ubo->putData(&camera_ubo_data, sizeof(CameraUBO));

    SceneLightsUBO light_ubo_data;
    light_ubo_data.light_count = m_lights.size();
    light_ubo_data.ambient_light = Eigen::Vector4f(0.1f, 0.1f, 0.1f, 1.0f);
    for (int i = 0; i < m_lights.size(); i++) {
        auto light = m_lights[i];
        auto lc = m_registry->getComponent<LightComponent>(light);
        auto tc = m_registry->getComponent<TransformComponent>(light);
        if (i >= MAX_LIGHTS)
            break;
        light_ubo_data.lights[i].position = tc->getPosition();
        light_ubo_data.lights[i].rotation = tc->getRotation();
        light_ubo_data.lights[i].color = lc->color;
        light_ubo_data.lights[i].distance_dropoff = lc->distance_dropoff;
        light_ubo_data.lights[i].type = static_cast<int>(lc->type);
    }
    m_light_ubo->putData(&light_ubo_data, sizeof(SceneLightsUBO));

    auto frustum = extractFrustumPlanes(proj_mat * view_mat);
    for (const auto& e : m_render_queue) {
        auto rc = m_registry->getComponent<RenderComponent>(e);
        auto tc = m_registry->getComponent<TransformComponent>(e);
        auto model = m_asset_bank->getAsset<Model>(rc->model);
        if (!model)
            continue;

        auto aabb = model->getBoundingBox();
        Eigen::Vector3f min = (tc->getModelMatrix() * Eigen::Vector4f(aabb.getMin().x(), aabb.getMin().y(), aabb.getMin().z(), 1.0)).head<3>();
        Eigen::Vector3f max = (tc->getModelMatrix() * Eigen::Vector4f(aabb.getMax().x(), aabb.getMax().y(), aabb.getMax().z(), 1.0)).head<3>();
        aabb = AABB(std::vector<Eigen::Vector3f>{min, max});
        if (!isAABBInFrustum(frustum, aabb)) {
            continue;
        }
        auto meshes = model->getMeshes();
        auto materials = model->getMaterialsIDs();
        auto nodes = model->getNodes();

        for (const auto& mtl : materials) {
            auto material = m_asset_bank->getAsset<Material>(mtl);
            auto shader = m_asset_bank->getAsset<Shader>(material->getShader());

            if (!shader) {
                continue;
            }
            shader->bind();
            for (int i = 0; i < model->getSkeleton().bones.size(); i++) {
                shader->loadMat4("finalBonesMatrices[" + std::to_string(i) + "]", model->getSkeleton().bones[i].finalTransformation);
            }
        }

        submitModel(0, nodes, meshes, materials, tc->getModelMatrix());
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

void ForwardRenderer::submitModel(int node_idx, const std::vector<Model::Node>& nodes, const std::vector<std::shared_ptr<Mesh>>& meshes,
                                  const std::vector<AssetUID>& materials, const Eigen::Matrix4f& transform) {
    auto node = nodes.at(node_idx);
    for (const auto& i : node.meshIndices) {
        if (i >= meshes.size()) {
            continue;
        }
        auto mtl_id = materials.at(i);
        auto mesh = meshes.at(i);
        auto material = m_asset_bank->getAsset<Material>(mtl_id);
        if (!material) {
            continue;
        }
        auto shader = m_asset_bank->getAsset<Shader>(material->getShader());
        if (!shader) {
            continue;
        }

        if (!mesh) {
            continue;
        }

        Eigen::Matrix4f node_transform;
        if (mesh->usesBones())
            node_transform = transform;
        else
            node_transform = transform * node.animatedTransform;

        std::unordered_map<AssetUID, std::shared_ptr<Texture>> texs;
        for (const auto& [name, value] : material->getAllUniforms()) {
            if (std::holds_alternative<AssetUID>(value)) {
                auto v = std::get<AssetUID>(value);
                if (auto tex = m_asset_bank->getAsset<Texture2D>(v); tex) {
                    texs.try_emplace(v, tex);
                }
            }
        }

        m_render_commands.push_back(RenderCommand{.mesh = mesh,
                                                  .material = material,
                                                  .shader = shader,
                                                  .textures = texs,
                                                  .model_matrix = node_transform,
                                                  .faceCulling = true,
                                                  .depthTest = true});
    }

    for (const auto& child_idx : node.children) {
        submitModel(child_idx, nodes, meshes, materials, transform);
    }
}

void ForwardRenderer::render() {
    m_geometry_pass.execute();
    auto result = m_geometry_pass.getResult();

    //Final pass, render the last result to the screen
    if (!this->target) {
        m_api->bindDefaultFramebuffer();
    } else {
        this->target->bind();
    }
    m_api->clear();
    auto shader = m_asset_bank->getAsset<Shader>(AssetPath::WithTypePrefix<Shader>("lastpass"));

    shader->bind();
    result->bindAttachment(0);
    shader->loadInt("uTexture", 0);
    m_quad_vao->bind();
    m_quad_vao->getIndexBuffer()->bind();
    m_api->renderVertexArray(m_quad_vao);
}

void ForwardRenderer::endFrame() {
    m_api->checkAndLogErrors();
    m_render_commands.clear();
    if (this->target != nullptr)
        this->target->unbind();
}

void ForwardRenderer::setTarget(const std::shared_ptr<Framebuffer>& fb) {
    this->target = fb;
}

void ForwardRenderer::resize(uint32_t width, uint32_t height) {
    if (target) {
        target->bind();
        target->resize(width, height);
    }
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
