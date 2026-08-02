#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GPUTexture.h>
#include <GraphicsFactory.h>
#include <ICEMath.h>
#include <Logger.h>
#include <Texture.h>

#include <Eigen/Dense>
#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "UIElement.h"

namespace ICE {

// One glyph's placement in the font atlas plus its metrics.
struct CharacterGlyph {
    Eigen::Vector2f uv_offset;  // top-left of the glyph's cell in the atlas (0..1)
    Eigen::Vector2f uv_scale;   // the cell's size in the atlas (0..1)
    Eigen::Vector2f size;       // glyph size in pixels
    Eigen::Vector2f bearing;    // offset from the baseline to the glyph's top-left, in pixels
    unsigned int advance = 0;   // pen advance to the next glyph, in 1/64 px
};

// A bitmap font rasterized by FreeType into a SINGLE atlas texture (not one GL texture per glyph,
// which cost ~128 texture objects and a bind per character). The UI shader is supplied per draw
// through the UIRenderContext, so the font owns no shader and no global state.
class Font {
   public:
    Font(const std::shared_ptr<GraphicsFactory>& factory, const std::string& font_path) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            Logger::Log(Logger::ERROR, "Text", "Could not init FreeType Library");
            return;
        }
        FT_Face face;
        if (FT_New_Face(ft, font_path.c_str(), 0, &face)) {
            Logger::Log(Logger::ERROR, "Text", "Failed to load font '%s'", font_path.c_str());
            FT_Done_FreeType(ft);
            return;
        }
        FT_Set_Pixel_Sizes(face, 0, kPixelSize);

        // Pass 1: rasterize every glyph and keep its coverage bitmap, measuring the atlas.
        struct Loaded {
            std::vector<uint8_t> pixels;
            int width = 0;
            int rows = 0;
            Eigen::Vector2f bearing{0, 0};
            unsigned int advance = 0;
        };
        std::unordered_map<char, Loaded> loaded;
        int atlas_width = 0;
        int atlas_height = 0;
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_DEFAULT) || FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
                Logger::Log(Logger::WARNING, "Text", "Failed to load glyph %d", static_cast<int>(c));
                continue;
            }
            const auto& bmp = face->glyph->bitmap;
            Loaded g;
            g.width = static_cast<int>(bmp.width);
            g.rows = static_cast<int>(bmp.rows);
            g.bearing = {static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top)};
            g.advance = static_cast<unsigned int>(face->glyph->advance.x);
            g.pixels.assign(bmp.buffer, bmp.buffer + static_cast<size_t>(g.width) * g.rows);
            atlas_width += g.width + kPadding;
            atlas_height = std::max(atlas_height, g.rows);
            loaded.emplace(static_cast<char>(c), std::move(g));
        }
        atlas_width = std::max(atlas_width, 1);
        atlas_height = std::max(atlas_height, 1);

        // Pass 2: blit each glyph into a single-channel buffer laid out left-to-right, top-aligned,
        // and record its atlas cell as normalized UVs.
        std::vector<uint8_t> atlas(static_cast<size_t>(atlas_width) * atlas_height, 0);
        int cursor_x = 0;
        for (auto& [c, g] : loaded) {
            for (int row = 0; row < g.rows; ++row) {
                for (int col = 0; col < g.width; ++col) {
                    atlas[static_cast<size_t>(row) * atlas_width + cursor_x + col] = g.pixels[static_cast<size_t>(row) * g.width + col];
                }
            }
            CharacterGlyph glyph;
            glyph.uv_offset = {static_cast<float>(cursor_x) / atlas_width, 0.0f};
            glyph.uv_scale = {static_cast<float>(g.width) / atlas_width, static_cast<float>(g.rows) / atlas_height};
            glyph.size = {static_cast<float>(g.width), static_cast<float>(g.rows)};
            glyph.bearing = g.bearing;
            glyph.advance = g.advance;
            m_glyphs.emplace(c, glyph);
            cursor_x += g.width + kPadding;
        }

        // MONO8 uploads with 1-byte row alignment, so the arbitrary atlas width is fine. The GL
        // texture ctor copies immediately, so the local buffer can go out of scope.
        Texture2D atlas_asset(atlas.data(), atlas_width, atlas_height, TextureFormat::MONO8);
        m_atlas = factory->createTexture2D(atlas_asset);

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    // Draw `text` with its top-left baseline near (x, y). `scale` maps glyph pixels to target
    // pixels (1.0 renders at the atlas's native size). Uses the caller's UI shader in text mode.
    void renderText(const std::string& text, float x, float y, float scale, const Eigen::Vector4f& color, const UIRenderContext& ctx) const {
        if (!m_atlas || !ctx.shader || !ctx.quad || !ctx.api) {
            return;
        }
        ctx.shader->bind();
        ctx.shader->loadInt("uMode", 2);  // text: sample the atlas .r as coverage
        ctx.shader->loadInt("uTexture", 0);
        ctx.shader->loadFloat4("uColor", color);
        ctx.shader->loadMat4("projection", ctx.projection);
        m_atlas->bind(0);
        ctx.quad->bind();
        ctx.quad->getIndexBuffer()->bind();

        for (char c : text) {
            auto it = m_glyphs.find(c);
            if (it == m_glyphs.end()) {
                continue;
            }
            const CharacterGlyph& g = it->second;
            const float xpos = x + g.bearing.x() * scale;
            const float ypos = y - g.bearing.y() * scale;
            const float w = g.size.x() * scale;
            const float h = g.size.y() * scale;
            if (w > 0 && h > 0) {
                ctx.shader->loadFloat2("uUVOffset", g.uv_offset);
                ctx.shader->loadFloat2("uUVScale", g.uv_scale);
                ctx.shader->loadMat4("model", transformationMatrix({xpos, ypos, 0}, {0, 0, 0}, {w, h, 0}));
                ctx.api->renderVertexArray(ctx.quad);
            }
            x += (g.advance >> 6) * scale;  // advance is in 1/64 px
        }
    }

    // Reference glyph height in pixels: a label of pixel-height H renders at scale H/glyphHeight().
    float glyphHeight() const {
        auto it = m_glyphs.find('W');
        return it != m_glyphs.end() ? it->second.size.y() : static_cast<float>(kPixelSize);
    }

   private:
    static constexpr int kPixelSize = 48;  // rasterization height
    static constexpr int kPadding = 1;     // 1px gutter between atlas cells to avoid bleed

    std::unordered_map<char, CharacterGlyph> m_glyphs;
    std::shared_ptr<GPUTexture> m_atlas;
};
}  // namespace ICE
