#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "UIElement.h"
#include "UIRect.h"

using namespace ICE;

namespace {
// UIRect owns no GPU resources (it draws through a UIRenderContext, never touched here), so the
// layout + hit-testing logic is exercised without a GL context.
std::unique_ptr<UIRect> rect(const std::string& name, Eigen::Vector2f pos, Eigen::Vector2f size) {
    return std::make_unique<UIRect>(name, pos, size, Eigen::Vector4f{1, 1, 1, 1});
}

// Counts the events an element receives.
struct EventCounts {
    int clicks = 0;
    int enters = 0;
    int leaves = 0;
    void attach(UIElement* e) {
        e->onEvent([this](const Event& ev) {
            switch (ev.type) {
                case EventType::Click: clicks++; break;
                case EventType::MouseEnter: enters++; break;
                case EventType::MouseLeave: leaves++; break;
            }
        });
    }
};

constexpr UIBound kViewport{0, 0, 800, 600};
}  // namespace

// The layout rule: a positive relative coordinate is a fraction from the near edge, size scales
// with the parent.
TEST(UIInteractionTest, ComputeBoundsResolvesRelativeToParent) {
    UIRect r("r", {0.25f, 0.5f}, {0.5f, 0.25f}, {1, 1, 1, 1});
    const UIBound b = r.computeBounds(kViewport);
    EXPECT_FLOAT_EQ(b.x, 200.0f);       // 0.25 * 800
    EXPECT_FLOAT_EQ(b.y, 300.0f);       // 0.50 * 600
    EXPECT_FLOAT_EQ(b.width, 400.0f);   // 0.50 * 800
    EXPECT_FLOAT_EQ(b.height, 150.0f);  // 0.25 * 600
}

// A negative relative coordinate anchors the element's far edge in from the parent's far edge.
TEST(UIInteractionTest, NegativeCoordinateAnchorsToFarEdge) {
    UIRect r("r", {-0.0f, -0.0f}, {0.25f, 0.25f}, {1, 1, 1, 1});
    // -0.0 is >= 0, so use a clearly-negative anchor instead:
    UIRect far_("far", {-0.1f, -0.2f}, {0.2f, 0.1f}, {1, 1, 1, 1});
    const UIBound b = far_.computeBounds(kViewport);
    // width 160 (0.2*800), anchored so its right edge is 0.1*800=80 in from the right: x = 800-80-160
    EXPECT_FLOAT_EQ(b.width, 160.0f);
    EXPECT_FLOAT_EQ(b.x, 800.0f - 80.0f - 160.0f);
    EXPECT_FLOAT_EQ(b.height, 60.0f);
    EXPECT_FLOAT_EQ(b.y, 600.0f - 120.0f - 60.0f);
}

// The acceptance: a click inside an element dispatches a Click event; outside does not.
TEST(UIInteractionTest, ClickInsideDispatchesEvent) {
    auto button = rect("button", {0.25f, 0.25f}, {0.5f, 0.5f});  // pixels [200,150]..[600,450]
    EventCounts counts;
    counts.attach(button.get());

    dispatchPointer(button.get(), kViewport, 400.0f, 300.0f, /*clicked=*/true);  // centre
    EXPECT_EQ(counts.clicks, 1);

    dispatchPointer(button.get(), kViewport, 10.0f, 10.0f, /*clicked=*/true);  // outside
    EXPECT_EQ(counts.clicks, 1);  // unchanged

    dispatchPointer(button.get(), kViewport, 400.0f, 300.0f, /*clicked=*/false);  // hover, no click
    EXPECT_EQ(counts.clicks, 1);
}

// MouseEnter fires once on entry, MouseLeave once on exit -- edges, not per-frame.
TEST(UIInteractionTest, HoverEdgesFireEnterAndLeaveOnce) {
    auto el = rect("el", {0.0f, 0.0f}, {0.5f, 0.5f});  // pixels [0,0]..[400,300]
    EventCounts counts;
    counts.attach(el.get());

    dispatchPointer(el.get(), kViewport, 100.0f, 100.0f, false);  // enter
    dispatchPointer(el.get(), kViewport, 150.0f, 150.0f, false);  // still inside
    EXPECT_EQ(counts.enters, 1);
    EXPECT_EQ(counts.leaves, 0);
    EXPECT_TRUE(el->hovered());

    dispatchPointer(el.get(), kViewport, 500.0f, 500.0f, false);  // leave
    dispatchPointer(el.get(), kViewport, 600.0f, 500.0f, false);  // still outside
    EXPECT_EQ(counts.enters, 1);
    EXPECT_EQ(counts.leaves, 1);
    EXPECT_FALSE(el->hovered());
}

// Children are laid out and hit-tested within the parent's bounds; a click hits the child it lands
// on, and both parent and child receive it when nested.
TEST(UIInteractionTest, NestedChildIsHitWithinParentBounds) {
    auto panel = rect("panel", {0.5f, 0.5f}, {0.5f, 0.5f});  // pixels [400,300]..[800,600]
    // child at the top-left quarter of the panel: panel-relative [0,0]..[0.5,0.5] -> [400,300]..[600,450]
    auto child = rect("child", {0.0f, 0.0f}, {0.5f, 0.5f});
    EventCounts panel_counts;
    EventCounts child_counts;
    panel_counts.attach(panel.get());
    child_counts.attach(child.get());
    panel->addChild(std::move(child));

    // Click at (450, 350): inside the child (and the panel).
    dispatchPointer(panel.get(), kViewport, 450.0f, 350.0f, true);
    EXPECT_EQ(child_counts.clicks, 1);
    EXPECT_EQ(panel_counts.clicks, 1);

    // Click at (700, 550): inside the panel but outside the child.
    dispatchPointer(panel.get(), kViewport, 700.0f, 550.0f, true);
    EXPECT_EQ(panel_counts.clicks, 2);
    EXPECT_EQ(child_counts.clicks, 1);  // unchanged
}

// A hidden element neither hovers nor clicks.
TEST(UIInteractionTest, InvisibleElementIsNotInteractive) {
    auto el = rect("el", {0.0f, 0.0f}, {1.0f, 1.0f});
    el->setVisible(false);
    EventCounts counts;
    counts.attach(el.get());

    dispatchPointer(el.get(), kViewport, 400.0f, 300.0f, true);
    EXPECT_EQ(counts.clicks, 0);
    EXPECT_EQ(counts.enters, 0);
}
