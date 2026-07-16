#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GPUTexture.h>
#include <GraphicsFactory.h>
#include <ICEMath.h>
#include <Logger.h>
#include <Shader.h>
#include <Texture.h>

#include <Eigen/Dense>

#include "GraphicsUtil.h"

namespace ICE {

struct CharacterGlyph {
  std::shared_ptr<GPUTexture> texture;  // ID handle of the glyph texture
  Eigen::Vector2f size;                 // Size of glyph
  Eigen::Vector2f bearing;              // Offset from baseline to left/top of glyph
  unsigned int advance;                 // Offset to advance to next glyph
};

class Font {
 public:
  Font(const std::shared_ptr<GraphicsFactory> &g_factory, const std::string &font_path) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
      Logger::Log(Logger::ERROR, "Text", "Could not init FreeType Library");
      return;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_path.c_str(), 0, &face)) {
      Logger::Log(Logger::ERROR, "Text", "Failed to load font '%s'", font_path.c_str());
      return;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);
    for (unsigned char c = 0; c < 128; c++) {
      // load character glyph
      if (FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
        Logger::Log(Logger::ERROR, "Text", "Failed to load Glyph");
        continue;
      }
      if (auto err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); err) {
        Logger::Log(Logger::ERROR, "Text", "Failed to render Glyph: %c with err %s", c, FT_Error_String(err));
        continue;
      }
      auto bitmap = face->glyph->bitmap;
      auto texture_asset = Texture2D(bitmap.buffer, bitmap.width, bitmap.rows, TextureFormat::MONO8);
      auto texture = g_factory->createTexture2D(texture_asset);

      // now store character for later use
      CharacterGlyph character = {texture,
                                  {face->glyph->bitmap.width, face->glyph->bitmap.rows},
                                  {face->glyph->bitmap_left, face->glyph->bitmap_top},
                                  face->glyph->advance.x};
      m_character.try_emplace(c, character);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    m_vao = GraphicsUtil::getNormalizedQuad(g_factory);
    if (!shader) {
      auto shader_asset =
          Shader({{ShaderStage::Vertex, {"", vertex_src}}, {ShaderStage::Fragment, {"", fragment_src}}});
      shader = g_factory->createShader(shader_asset);
    }
  }

  void renderText(std::string text, float x, float y, float scale, const Eigen::Vector4f &color,
                  const Eigen::Matrix4f &projection, const std::shared_ptr<RendererAPI> &api) {
    shader->bind();
    shader->loadInt("uTexture", 0);
    shader->loadFloat4("uColor", color);
    shader->loadMat4("projection", projection);
    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
      CharacterGlyph ch = m_character[*c];

      float xpos = x + ch.bearing.x() * scale;
      float ypos = y - ch.bearing.y() * scale;

      float w = ch.size.x() * scale;
      float h = ch.size.y() * scale;
      // render glyph texture over quad
      ch.texture->bind(0);
      Eigen::Matrix4f model = transformationMatrix({xpos, ypos, 0}, {0, 0, 0}, {w, h, 0});

      shader->loadMat4("model", model);
      m_vao->bind();
      m_vao->getIndexBuffer()->bind();
      api->renderVertexArray(m_vao);

      // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
      x += (ch.advance >> 6) * scale;  // bitshift by 6 to get value in pixels (2^6 = 64)
    }
  }

  float horizontal_norm() { return m_character['W'].size.y(); }

 private:
  std::unordered_map<char, CharacterGlyph> m_character;
  std::shared_ptr<VertexArray> m_vao;
  static inline std::shared_ptr<ShaderProgram> shader;
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
      "void main() {"
      "frag_color = vec4(uColor.xyz, uColor.w * texture(uTexture, fUV).r);"
      "}";
};
}  // namespace ICE
