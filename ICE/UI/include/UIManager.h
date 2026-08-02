#pragma once

#include <GraphicsAPI.h>
#include <GraphicsFactory.h>
#include <ShaderProgram.h>

#include <memory>
#include <string>
#include <vector>

#include "Font.h"
#include "UIElement.h"

namespace ICE {

// Owns the UI: the root elements, the shared shader (loaded as an asset by the engine, not inline
// GLSL), the shared quad, and the font atlas. It lays elements out, draws them, and hit-tests them
// against pointer input. This replaces the transplanted module's global statics and stub UI class.
//
// Rendering runs at present time as a render-graph pass (see UIRenderPass) so the UI composites
// over the finished scene; input is fed in as primitives (see processInput) so this module needs no
// input-service dependency and the hit-testing is unit-testable without a GL context.
class UIManager {
   public:
    // ui_shader is the "ui" shader asset resolved by the engine; font_path points at a .ttf.
    UIManager(const std::shared_ptr<GraphicsFactory>& factory, const std::shared_ptr<ShaderProgram>& ui_shader,
              const std::string& font_path);

    // Adopt a top-level element and return a borrowed pointer for further setup.
    UIElement* add(std::unique_ptr<UIElement> element);

    // The shared font, for constructing UILabels.
    const std::shared_ptr<Font>& font() const { return m_font; }

    // Draw all visible elements into the currently bound target, sized to (width, height).
    void render(RendererAPI* api, int width, int height);

    // Hit-test the pointer against the tree and dispatch Click/MouseEnter/MouseLeave. `clicked` is
    // the primary-button press EDGE for this frame (true only on the frame it goes down). Taking
    // primitives rather than the InputManager keeps the UI backend- and input-service-agnostic and
    // makes this directly testable.
    void processInput(float mouse_x, float mouse_y, bool clicked, int width, int height);

    const std::vector<std::unique_ptr<UIElement>>& elements() const { return m_root; }

   private:
    std::shared_ptr<GraphicsFactory> m_factory;
    std::shared_ptr<ShaderProgram> m_shader;
    std::shared_ptr<VertexArray> m_quad;
    std::shared_ptr<Font> m_font;
    std::vector<std::unique_ptr<UIElement>> m_root;
};
}  // namespace ICE
