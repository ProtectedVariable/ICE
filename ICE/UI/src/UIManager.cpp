#include "UIManager.h"

#include <GraphicsUtil.h>

namespace ICE {
namespace {

// Orthographic projection mapping pixel coordinates with a TOP-LEFT origin (x right, y down) to
// clip space: x in [0,w] -> [-1,1], y in [0,h] -> [1,-1]. Matches the top-left UIBound convention.
Eigen::Matrix4f orthoTopLeft(float w, float h) {
    Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
    m(0, 0) = 2.0f / w;
    m(0, 3) = -1.0f;
    m(1, 1) = -2.0f / h;
    m(1, 3) = 1.0f;
    m(2, 2) = -1.0f;  // near -1, far 1
    return m;
}
}  // namespace

UIManager::UIManager(const std::shared_ptr<GraphicsFactory>& factory, const std::shared_ptr<ShaderProgram>& ui_shader,
                     const std::string& font_path)
    : m_factory(factory),
      m_shader(ui_shader),
      m_quad(GraphicsUtil::getNormalizedQuad(factory)),
      m_font(std::make_shared<Font>(factory, font_path)) {}

UIElement* UIManager::add(std::unique_ptr<UIElement> element) {
    UIElement* raw = element.get();
    m_root.push_back(std::move(element));
    return raw;
}

void UIManager::render(RendererAPI* api, int width, int height) {
    if (!api || width <= 0 || height <= 0 || !m_shader || !m_quad) {
        return;
    }
    UIRenderContext ctx;
    ctx.api = api;
    ctx.shader = m_shader;
    ctx.quad = m_quad;
    ctx.projection = orthoTopLeft(static_cast<float>(width), static_cast<float>(height));

    // UI draws as a flat overlay: no depth test, alpha blending on for text coverage and
    // translucent panels, and NO backface culling. Culling matters because the orthographic
    // projection flips Y (top-left origin), which reverses the quad's winding to clockwise -- a
    // back face. The geometry pass leaves GL_CULL_FACE enabled, so without this every UI quad is
    // silently culled and nothing appears.
    api->setDepthTest(false);
    api->setBlend(true);
    api->setBackfaceCulling(false);

    const UIBound root{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};
    for (const auto& element : m_root) {
        if (element->isVisible()) {
            element->render(root, ctx);
        }
    }
}

void UIManager::processInput(float mouse_x, float mouse_y, bool clicked, int width, int height) {
    const UIBound root{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};
    for (const auto& element : m_root) {
        dispatchPointer(element.get(), root, mouse_x, mouse_y, clicked);
    }
}
}  // namespace ICE
