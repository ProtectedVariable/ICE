
#define IMGUI_DEFINE_MATH_OPERATORS

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <GLFWWindow.h>
#include <ICEEngine.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_glfw.h>
#include <ImGUI/imgui_impl_opengl3.h>
#include <ImGUI/imgui_internal.h>
#include <OpenGLFactory.h>

#include <fstream>
#include <iostream>
#include <string>

#include "Editor.h"
#include "ProjectSelection.h"

enum class UIState { PROJECT_SELECTION, EDITOR };

class Iceberg {
   public:
    Iceberg(const std::shared_ptr<ICE::GLFWWindow>& window) : m_window(window), m_controller(std::make_unique<ProjectSelection>(m_engine)) {
        m_engine->initialize(std::make_shared<ICE::OpenGLFactory>(), window);
    }

    void loop() {
        while (!m_window->shouldClose()) {
            IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            glClear(GL_COLOR_BUFFER_BIT);

            if (m_controller->update()) {
                if (m_state == UIState::PROJECT_SELECTION) {
                    m_controller = std::make_unique<Editor>(m_engine, std::make_shared<ICE::OpenGLFactory>());
                    m_state = UIState::EDITOR;
                }
            }

            if (m_state == UIState::EDITOR) {
                m_engine->step();
            }
            ImGui::ShowDemoWindow();

            ImGui::Render();

            m_engine->getApi()->bindDefaultFramebuffer();
            int display_w, display_h;
            m_window->getFramebufferSize(&display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            m_window->swapBuffers();
        }
        if (m_engine->getProject()) {
            m_engine->getConfig().getLocalProjects()->push_back(*m_engine->getProject());
            m_engine->getProject()->writeToFile(m_engine->getCamera());
            m_engine->getConfig().save();
        }
    }

   private:
    std::shared_ptr<ICE::GLFWWindow> m_window;
    std::shared_ptr<ICE::ICEEngine> m_engine = std::make_shared<ICE::ICEEngine>();
    std::unique_ptr<Controller> m_controller;
    UIState m_state = UIState::PROJECT_SELECTION;
};

int main(int argc, char const* argv[]) {
    auto window = std::make_shared<ICE::GLFWWindow>(1280, 720, "ICE Engine v0.2");
    ICE::OpenGLFactory g_factory;
    auto context = g_factory.createContext(window);
    context->initialize();
    window->setSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window->getHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Iceberg iceberg(window);

    iceberg.loop();

    return 0;
}
