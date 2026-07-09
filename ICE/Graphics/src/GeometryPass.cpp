#include "GeometryPass.h"

#include "InstanceData.h"

namespace ICE {
GeometryPass::GeometryPass(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory, const FrameBufferFormat& format)
    : m_api(api),
      m_factory(factory) {
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

    for (const auto& command : *m_render_queue) {
        auto& shader = command.shader;
        auto& material = command.material;
        auto& mesh = command.mesh;

        m_api->setBackfaceCulling(command.faceCulling);
        m_api->setDepthTest(command.depthTest);
        m_api->setDepthMask(command.depthWrite);
        m_api->setDepthFunc(command.depth_func);

        if (shader != current_shader) {
            shader->bind();
            current_shader = shader;
        }

        // Handle bone matrices (non-instanced only)
        if (!command.is_instanced && command.bones && !command.bones->empty()) {
            // TODO: Use UBO instead of individual uploads for better performance
            for (const auto& [id, matrix] : *command.bones) {
                current_shader->loadMat4("bonesTransformMatrices[" + std::to_string(id) + "]", matrix);
            }
        }

        if (material != current_material) {
            auto& textures = command.textures;
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
                    auto v = std::get<AssetUID>(value);
                    if (textures->contains(v)) {
                        auto& tex = textures->at(v);
                        if (tex) {
                            tex->bind(texture_count);
                            shader->loadInt(name, texture_count);
                            texture_count++;
                        }
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
}

std::shared_ptr<Framebuffer> GeometryPass::getResult() const {
    return m_framebuffer;
}

void GeometryPass::resize(int w, int h) {
    m_framebuffer->resize(w, h);
}

}  // namespace ICE