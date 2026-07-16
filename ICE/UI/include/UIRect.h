#pragma once

#include <GraphicsFactory.h>

#include "GraphicsUtil.h"
#include "UIElement.h"

namespace ICE {
class UIRect : public UIElement {
 public:
  UIRect(const std::string &name, const Eigen::Vector2f &position, const Eigen::Vector2f &size,
         const Eigen::Vector4f &color, const std::shared_ptr<ICE::GraphicsFactory> &g_factory)
      : UIElement(name, position, size),
        m_color(color) {
    if (shader == nullptr) {
      auto shader_asset =
          Shader({{ShaderStage::Vertex, {"", vertex_src}}, {ShaderStage::Fragment, {"", fragment_src}}});
      shader = g_factory->createShader(shader_asset);
      vao = GraphicsUtil::getNormalizedQuad(g_factory);
    }
  }

  void render(const UIBound &absolute_bounds, const Eigen::Matrix4f &projection,
              const std::shared_ptr<RendererAPI> &api) override {
    Eigen::Vector2f abs_size =
        m_relative_size.cwiseProduct(Eigen::Vector2f{absolute_bounds.width, absolute_bounds.height});

    Eigen::Vector2f abs_pos(0, 0);
    if (m_relative_pos.x() >= 0) {
      abs_pos.x() = m_relative_pos.x() * absolute_bounds.width + absolute_bounds.x;
    } else {
      abs_pos.x() = (1 + m_relative_pos.x()) * absolute_bounds.width + absolute_bounds.x - abs_size.x();
    }

    if (m_relative_pos.y() >= 0) {
      abs_pos.y() = m_relative_pos.y() * absolute_bounds.height + absolute_bounds.y;
    } else {
      abs_pos.y() = (1 + m_relative_pos.y()) * absolute_bounds.height + absolute_bounds.y - abs_size.y();
    }

    Eigen::Matrix4f model =
        ICE::transformationMatrix({abs_pos.x(), abs_pos.y(), 0}, {0, 0, 0}, {abs_size.x(), abs_size.y(), 0});

    shader->bind();
    shader->loadInt("uUseTexture", m_texture ? 1 : 0);
    shader->loadFloat4("uColor", m_color);
    shader->loadMat4("model", model);
    shader->loadMat4("projection", projection);

    if (m_texture) {
      m_texture->bind(0);
    }

    vao->bind();
    vao->getIndexBuffer()->bind();
    api->renderVertexArray(vao);

    for (const auto &child : m_children) {
      child->render({abs_pos.x(), abs_pos.y(), abs_size.x(), abs_size.y()}, projection, api);
    }
  }

  void setTexture(const std::shared_ptr<ICE::GPUTexture> &texture) { m_texture = texture; }

  void update() override {}

 private:
  Eigen::Vector4f m_color;
  std::shared_ptr<ICE::GPUTexture> m_texture = nullptr;

  inline static std::shared_ptr<ShaderProgram> shader;
  inline static std::shared_ptr<VertexArray> vao;
  inline static std::string vertex_src =
      "#version 330 core\n"
      "layout (location = 0) in vec2 aPos;\n"
      "out vec2 fUV;\n"
      "uniform mat4 projection;\n"
      "uniform mat4 model;\n"
      "void main() {"
      "gl_Position = projection * model * vec4(aPos, 0.0, 1.0);"
      "fUV = aPos;"
      "}";
  inline static std::string fragment_src =
      "#version 330 core\n"
      "out vec4 frag_color;\n"
      "in vec2 fUV;\n"
      "uniform vec4 uColor;\n"
      "uniform sampler2D uTexture;\n"
      "uniform bool uUseTexture;\n"
      "void main() {"
      "frag_color = uColor;"
      "if(uUseTexture) { frag_color *= texture(uTexture, fUV); }"
      "}";
};
}  // namespace ICE