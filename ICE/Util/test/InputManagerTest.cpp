#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include <InputManager.h>
#include <Window.h>

using namespace ICE;

namespace {
// Mock handlers expose the stored listeners so a test can fire events the way GLFW would during
// pollEvents (the callback vectors are protected on the base classes).
class MockKeyboardHandler : public KeyboardHandler {
   public:
    void firePress(Key k) {
        for (auto& cb : m_keypress_callbacks) cb(k);
    }
    void fireRelease(Key k) {
        for (auto& cb : m_keyrelease_callbacks) cb(k);
    }
};

class MockMouseHandler : public MouseHandler {
   public:
    void setGrabMouse(bool) override {}
    void fireMove(float x, float y) {
        for (auto& cb : m_mousemove_callbacks) cb(x, y);
    }
    void fireButton(MouseButton b, ButtonAction a) {
        for (auto& cb : m_mousebutton_callbacks) cb(b, a);
    }
};

class MockWindow : public Window {
   public:
    MockWindow() : m_mouse(std::make_shared<MockMouseHandler>()), m_keyboard(std::make_shared<MockKeyboardHandler>()) {}
    void* getHandle() const override { return nullptr; }
    bool shouldClose() override { return false; }
    void close() override {}
    std::pair<std::shared_ptr<MouseHandler>, std::shared_ptr<KeyboardHandler>> getInputHandlers() const override {
        return {m_mouse, m_keyboard};
    }
    void pollEvents() override {}
    void swapBuffers() override {}
    void getFramebufferSize(int* w, int* h) override {
        *w = 800;
        *h = 600;
    }
    void setSwapInterval(int) override {}
    void makeContextCurrent() override {}
    void setResizeCallback(const WindowResizeCallback&) override {}
    std::pair<int, int> getSize() const override { return {800, 600}; }

    std::shared_ptr<MockMouseHandler> m_mouse;
    std::shared_ptr<MockKeyboardHandler> m_keyboard;
};

struct InputFixture {
    std::shared_ptr<MockWindow> window = std::make_shared<MockWindow>();
    InputManager input{window};
    MockMouseHandler& mouse() { return *window->m_mouse; }
    MockKeyboardHandler& keyboard() { return *window->m_keyboard; }
};
}  // namespace

// PRESS is a one-frame edge: visible the frame the key goes down, gone after the frame's update().
TEST(InputManagerTest, PressLastsExactlyOneFrame) {
    InputFixture f;
    f.keyboard().firePress(Key::KEY_W);  // event fires during pollEvents, before step()'s update()
    EXPECT_EQ(f.input.getKeyAction(Key::KEY_W), KeyAction::PRESS);
    EXPECT_TRUE(f.input.isKeyDown(Key::KEY_W));

    f.input.update(0.016f);  // end of frame

    EXPECT_EQ(f.input.getKeyAction(Key::KEY_W), KeyAction::NONE);  // edge consumed
    EXPECT_TRUE(f.input.isKeyDown(Key::KEY_W));                    // ...but still held
}

TEST(InputManagerTest, ReleaseEdgeAndState) {
    InputFixture f;
    f.keyboard().firePress(Key::KEY_A);
    f.input.update(0.016f);
    EXPECT_TRUE(f.input.isKeyDown(Key::KEY_A));

    f.keyboard().fireRelease(Key::KEY_A);
    EXPECT_EQ(f.input.getKeyAction(Key::KEY_A), KeyAction::RELEASE);
    EXPECT_FALSE(f.input.isKeyDown(Key::KEY_A));

    f.input.update(0.016f);
    EXPECT_EQ(f.input.getKeyAction(Key::KEY_A), KeyAction::NONE);
}

// No spurious jump on the first cursor sample.
TEST(InputManagerTest, MouseDeltaFirstFrameIsZero) {
    InputFixture f;
    f.mouse().fireMove(100.0f, 50.0f);
    Eigen::Vector2f d = f.input.getMouseDelta();
    EXPECT_FLOAT_EQ(d.x(), 0.0f);
    EXPECT_FLOAT_EQ(d.y(), 0.0f);
}

// Delta is the movement across the frame, and resets after update() rolls the previous position.
TEST(InputManagerTest, MouseDeltaIsFrameToFrame) {
    InputFixture f;
    f.mouse().fireMove(100.0f, 50.0f);  // first sample: delta 0, prev seeded
    f.input.update(0.016f);             // prev = (100, 50)

    f.mouse().fireMove(110.0f, 45.0f);  // moved (+10, -5)
    Eigen::Vector2f d = f.input.getMouseDelta();
    EXPECT_FLOAT_EQ(d.x(), 10.0f);
    EXPECT_FLOAT_EQ(d.y(), -5.0f);

    f.input.update(0.016f);  // prev catches up
    Eigen::Vector2f d2 = f.input.getMouseDelta();
    EXPECT_FLOAT_EQ(d2.x(), 0.0f);
    EXPECT_FLOAT_EQ(d2.y(), 0.0f);
}
