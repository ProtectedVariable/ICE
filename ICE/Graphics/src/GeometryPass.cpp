#include "GeometryPass.h"

#include <algorithm>

#include "InstanceData.h"

namespace ICE {
GeometryPass::GeometryPass(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory,
                           const std::shared_ptr<GPURegistry>& gpu_registry, const FrameBufferFormat& format)
    : m_api(api),
      m_factory(factory),
      m_gpu_registry(gpu_registry) {
    m_framebuffer = factory->createFramebuffer(format);
    m_instance_buffer = factory->createVertexBuffer();
}

void GeometryPass::execute() {
    m_framebuffer->bind();
    m_api->setViewport(0, 0, m_framebuffer->getFormat().width, m_framebuffer->getFormat().height);
    m_api->clear();
    ShaderProgram* current_shader = nullptr;
    Material* current_material = nullptr;
    GPUMesh* current_mesh = nullptr;

    // Cache render state within the pass so identical consecutive commands (e.g. a run of
    // opaque draws) don't re-issue the same GL state calls. Forced on the first command.
    bool state_init = false;
    bool cur_cull = false, cur_depth_test = false, cur_depth_write = false, cur_blend = false;
    DepthFunc cur_depth_func = DepthFunc::Less;

    for (const auto& command : *m_render_queue) {
        auto& shader = command.shader;
        auto& material = command.material;
        auto& mesh = command.mesh;

        if (!state_init || cur_cull != command.faceCulling) {
            m_api->setBackfaceCulling(command.faceCulling);
            cur_cull = command.faceCulling;
        }
        if (!state_init || cur_depth_test != command.depthTest) {
            m_api->setDepthTest(command.depthTest);
            cur_depth_test = command.depthTest;
        }
        if (!state_init || cur_depth_write != command.depthWrite) {
            m_api->setDepthMask(command.depthWrite);
            cur_depth_write = command.depthWrite;
        }
        if (!state_init || cur_depth_func != command.depth_func) {
            m_api->setDepthFunc(command.depth_func);
            cur_depth_func = command.depth_func;
        }
        if (!state_init || cur_blend != command.blend) {
            m_api->setBlend(command.blend);
            cur_blend = command.blend;
        }
        state_init = true;

        if (shader != current_shader) {
            shader->bind();
            current_shader = shader;
        }

        // Handle bone matrices (non-instanced only). Pack into a contiguous, id-indexed
        // buffer (reused across draws) and upload the whole palette in one glUniformMatrix4fv
        // call instead of one string-built uniform lookup + upload per bone.
        if (!command.is_instanced && command.bones && !command.bones->empty()) {
            int max_id = 0;
            for (const auto& [id, matrix] : *command.bones) {
                max_id = std::max(max_id, id);
            }
            m_bone_palette.assign(static_cast<size_t>(max_id) + 1, Eigen::Matrix4f::Identity());
            for (const auto& [id, matrix] : *command.bones) {
                if (id >= 0) {
                    m_bone_palette[id] = matrix;
                }
            }
            current_shader->loadMat4v("bonesTransformMatrices", m_bone_palette.data(), static_cast<uint32_t>(m_bone_palette.size()));
        }

        // Skybox commands carry a null material (their uniforms come from the skybox shader),
        // so guard against it; opaque/transparent geometry always has one.
        if (material && material != current_material) {
            current_material = material;
            int texture_count = 0;

            //TODO: Can we do better ?
            for (const auto& [name, value] : material->getAllUniforms()) {
                if (std::holds_alternative<float>(value)) {
                    auto v = std::get<float>(value);
                    shader->loadFloat(name, v);
                } else if (!std::holds_alternative<AssetUID>(value) && std::holds_alternative<int>(value)) {
                    auto v = std::get<int>(value);
                    shader->loadInt(name, v);
                } else if (std::holds_alternative<AssetUID>(value)) {
                    // Resolve the texture from its AssetUID to a raw GPU pointer here, at bind time
                    // (uploading on first use), instead of carrying a per-command shared_ptr map.
                    auto v = std::get<AssetUID>(value);
                    if (GPUTexture* tex = m_gpu_registry->texture2DPtr(v)) {
                        tex->bind(texture_count);
                        shader->loadInt(name, texture_count);
                        texture_count++;
                    }
                } else if (std::holds_alternative<Eigen::Vector2f>(value)) {
                    auto& v = std::get<Eigen::Vector2f>(value);
                    shader->loadFloat2(name, v);
                } else if (std::holds_alternative<Eigen::Vector3f>(value)) {
                    auto& v = std::get<Eigen::Vector3f>(value);
                    shader->loadFloat3(name, v);
                } else if (std::holds_alternative<Eigen::Vector4f>(value)) {
                    auto& v = std::get<Eigen::Vector4f>(value);
                    shader->loadFloat4(name, v);
                } else if (std::holds_alternative<Eigen::Matrix4f>(value)) {
                    auto& v = std::get<Eigen::Matrix4f>(value);
                    shader->loadMat4(name, v);
                } else {
                    throw std::runtime_error("Uniform type not implemented");
                }
            }
        }

        if (current_mesh != mesh) {
            current_mesh = mesh;
            auto va = mesh->getVertexArray();
            va->bind();
            va->getIndexBuffer()->bind();
        }

        // Reuse the pass-owned instance buffer: upload this command's per-instance data
        // and (re)bind it at attribute slot 7 of the current mesh's vertex array.
        auto va = mesh->getVertexArray();
        if (command.is_instanced && command.instance_data) {
            m_instance_buffer->putData(command.instance_data->data(), command.instance_count * sizeof(InstanceData));
        } else {
            m_instance_buffer->putData(command.model_matrix.data(), sizeof(Eigen::Matrix4f));
        }
        va->pushVertexBuffer(m_instance_buffer, 7, 16, 1);
        m_api->renderVertexArrayInstanced(va, command.instance_count);
    }

    // Restore the globally-on blend state expected by other passes (final blit, editor
    // picking), since opaque commands turned it off.
    m_api->setBlend(true);
}

std::shared_ptr<Framebuffer> GeometryPass::getResult() const {
    return m_framebuffer;
}

void GeometryPass::resize(int w, int h) {
    m_framebuffer->resize(w, h);
}

}  // namespace ICE