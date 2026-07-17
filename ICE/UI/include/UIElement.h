#pragma once

#include <GraphicsAPI.h>
#include <ShaderProgram.h>
#include <VertexArray.h>

#include <Eigen/Dense>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ICE {

enum class EventType { Click, MouseEnter, MouseLeave };

struct Event {
    EventType type;
    int button = 0;
};

// An axis-aligned rectangle in pixels (origin top-left of the target).
struct UIBound {
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
};

// Everything a UI element needs to draw, owned by the UIManager and passed down each frame.
// Elements therefore hold no GPU resources of their own -- no per-class `static` shader/VAO, which
// was the transplanted module's global-state smell.
struct UIRenderContext {
    RendererAPI* api = nullptr;
    std::shared_ptr<ShaderProgram> shader;  // the UI shader (uMode/uColor/uTexture/uUV*)
    std::shared_ptr<VertexArray> quad;      // the shared normalized [0,1] quad
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
};

// Base class for UI widgets. Position and size are RELATIVE to the parent's bounds ([0,1] fractions;
// a negative coordinate anchors from the far/right/bottom edge), resolved to absolute pixels by
// computeBounds() -- the single layout rule shared by rendering and hit-testing.
class UIElement {
   public:
    using EventCallback = std::function<void(const Event&)>;

    UIElement(const std::string& name, const Eigen::Vector2f& position, const Eigen::Vector2f& size)
        : m_name(name),
          m_relative_pos(position),
          m_relative_size(size) {}
    virtual ~UIElement() = default;

    // Draw this element (and its children) into ctx, given the parent's absolute bounds.
    virtual void render(const UIBound& parent_bounds, const UIRenderContext& ctx) = 0;
    virtual void update() {}

    // Resolve this element's absolute pixel bounds within `parent`. The one layout rule: a positive
    // relative coordinate is a fraction from the parent's near edge; a negative one anchors the
    // element's far edge that fraction in from the parent's far edge.
    UIBound computeBounds(const UIBound& parent) const {
        const Eigen::Vector2f abs_size = m_relative_size.cwiseProduct(Eigen::Vector2f{parent.width, parent.height});
        Eigen::Vector2f abs_pos;
        abs_pos.x() = m_relative_pos.x() >= 0 ? m_relative_pos.x() * parent.width + parent.x
                                              : (1 + m_relative_pos.x()) * parent.width + parent.x - abs_size.x();
        abs_pos.y() = m_relative_pos.y() >= 0 ? m_relative_pos.y() * parent.height + parent.y
                                              : (1 + m_relative_pos.y()) * parent.height + parent.y - abs_size.y();
        return {abs_pos.x(), abs_pos.y(), abs_size.x(), abs_size.y()};
    }

    static bool contains(const UIBound& b, float px, float py) {
        return px >= b.x && px <= b.x + b.width && py >= b.y && py <= b.y + b.height;
    }

    void addChild(std::unique_ptr<UIElement>&& child) {
        child->m_parent = this;
        m_children.push_back(std::move(child));
    }

    const std::string& getName() const { return m_name; }
    bool isVisible() const { return m_visible; }
    const std::vector<std::unique_ptr<UIElement>>& getChildren() const { return m_children; }

    void setPosition(const Eigen::Vector2f& position) { m_relative_pos = position; }
    void setSize(const Eigen::Vector2f& size) { m_relative_size = size; }
    void setVisible(bool visible) { m_visible = visible; }

    // Interaction. The UIManager hit-tests each frame and calls dispatch() with Click/MouseEnter/
    // MouseLeave; register a handler with onEvent(). hovered() is the manager's tracked state.
    void onEvent(EventCallback cb) { m_on_event = std::move(cb); }
    void dispatch(const Event& e) const {
        if (m_on_event) {
            m_on_event(e);
        }
    }
    bool hovered() const { return m_hovered; }
    void setHovered(bool h) { m_hovered = h; }

   protected:
    std::string m_name;
    Eigen::Vector2f m_relative_pos;
    Eigen::Vector2f m_relative_size;
    bool m_visible = true;
    bool m_hovered = false;
    EventCallback m_on_event;
    UIElement* m_parent = nullptr;
    std::vector<std::unique_ptr<UIElement>> m_children;
};

// Hit-test `element` (and its children) against the pointer within `parent`, updating hover state
// and dispatching Click/MouseEnter/MouseLeave. `clicked` is the primary-button press edge for the
// frame. Pure layout/geometry -- no GPU resources -- so it is exercised by the UI tests directly.
inline void dispatchPointer(UIElement* element, const UIBound& parent, float mouse_x, float mouse_y, bool clicked) {
    const UIBound bounds = element->computeBounds(parent);
    const bool inside = element->isVisible() && UIElement::contains(bounds, mouse_x, mouse_y);

    if (inside && !element->hovered()) {
        element->setHovered(true);
        element->dispatch({EventType::MouseEnter, 0});
    } else if (!inside && element->hovered()) {
        element->setHovered(false);
        element->dispatch({EventType::MouseLeave, 0});
    }
    if (inside && clicked) {
        element->dispatch({EventType::Click, 0});
    }

    for (const auto& child : element->getChildren()) {
        dispatchPointer(child.get(), bounds, mouse_x, mouse_y, clicked);
    }
}
}  // namespace ICE
