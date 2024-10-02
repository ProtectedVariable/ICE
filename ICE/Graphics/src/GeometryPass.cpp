#include "GeometryPass.h"

namespace ICE {
GeometryPass::GeometryPass(const std::shared_ptr<RendererAPI> &api, const GraphicsFactory& factory, const FrameBufferFormat& format) : m_api(api) {
    m_framebuffer = factory.createFramebuffer(format);
}

void GeometryPass::execute() {
    m_framebuffer->bind();
    std::shared_ptr<Shader> current_shader;
    std::shared_ptr<Material> current_material;
    std::shared_ptr<Mesh> current_mesh;
    for (const auto& command : m_render_queue.value()) {
        auto& shader = command.shader;
        auto& material = command.material;
        auto& mesh = command.mesh;

        m_api->setBackfaceCulling(command.faceCulling);
        m_api->setDepthTest(command.depthTest);

        if(shader != current_shader) {
            shader->bind();
            current_shader = shader;
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
                    auto& tex = textures.at(v);
                    tex->bind(texture_count);
                    shader->loadInt(name, texture_count);
                    texture_count++;
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

        shader->loadMat4("model", command.model_matrix);
        m_api->renderVertexArray(mesh->getVertexArray());
    }
}

}  // namespace ICE