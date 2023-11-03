
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
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
    Iceberg(GLFWwindow* window) : m_window(window), m_controller(std::make_unique<ProjectSelection>(m_engine)) {
        m_engine->initialize(std::make_shared<ICE::OpenGLFactory>(), (void*) window);
    }

    void loop() {
        while (!glfwWindowShouldClose(m_window)) {
            IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            glClear(GL_COLOR_BUFFER_BIT);

            if (m_controller->update()) {
                if (m_state == UIState::PROJECT_SELECTION) {
                    m_controller = std::make_unique<Editor>(m_engine);
                    m_state = UIState::EDITOR;
                }
            }

            if (m_state == UIState::EDITOR) {
                m_engine->step(m_engine->getProject()->getCurrentScene());
            }
            ImGui::ShowDemoWindow();
            ImGui::Render();

            int display_w, display_h;
            glfwGetFramebufferSize(m_window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(m_window);
        }
        if (m_engine->getProject()) {
            m_engine->getConfig().getLocalProjects()->push_back(*m_engine->getProject());
            m_engine->getProject()->writeToFile(m_engine->getCamera());
            m_engine->getConfig().save();
        }
    }

   private:
    GLFWwindow* m_window;
    std::shared_ptr<ICE::ICEEngine> m_engine = std::make_shared<ICE::ICEEngine>();
    std::unique_ptr<Controller> m_controller;
    UIState m_state = UIState::PROJECT_SELECTION;
};

int main(int argc, char const* argv[]) {
    if (!glfwInit())
        return 1;

        // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ICE Engine", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    bool err = gl3wInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    Iceberg iceberg(window);

    iceberg.loop();

    return 0;
}
