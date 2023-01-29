//
// Created by Thomas Ibanez on 27.11.20.
//
#include <Util/Logger.h>
#include <Core/ICEEngine.h>
#include <Platform/FileUtils.h>
#include <Util/OBJLoader.h>
#include "ICEGUI.h"
#include "HierarchyPane.h"
#include "InspectorPane.h"
#include "AssetPane.h"

#include <GL/gl3w.h>            // Initialize with gl3wInit()
// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>
#include <ImGUI/imgui_impl_glfw.h>
#include <ImGUI/imgui_impl_opengl3.h>
#include <ImGUI/ImGuizmo.h>
#include <IO/Project.h>

#include <string>
#include <iostream>
#include <fstream>

#define CAMERA_DELTA 1.f

namespace ICE {
    int gui_init = 0;

    void ICEGUI::render() {
        if(engine->getProject() != nullptr) {

            ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
            flags |= ImGuiWindowFlags_NoDocking;
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Main", 0, flags);
            ImGui::PopStyleVar();

            if (ImGui::BeginMenuBar())
            {
                if(ImGui::BeginMenu("File"))
                {
                    if(ImGui::MenuItem("New Scene")) {
                        showNewScenePopup = true;
                    }
                    if(ImGui::MenuItem("Load Scene")) {
                        showLoadScenePopup = true;
                    }
                    ImGui::Separator();
                    ImGui::MenuItem("Save Project");
                    ImGui::Separator();
                    if(ImGui::BeginMenu("Import...")) {
                        if(ImGui::MenuItem("Mesh (.obj)")) {
                            engine->importMesh();
                        }
                        if(ImGui::MenuItem("2D Texture")) {
                            engine->importTexture(false);
                        }
                        if(ImGui::MenuItem("CubeMap Texture")) {
                            engine->importTexture(true);
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            if(showNewScenePopup) {
                ImGui::OpenPopup("Create Scene...");
            }

            if(showLoadScenePopup) {
                ImGui::OpenPopup("Load Scene...");
            }

            if(ImGui::BeginPopupModal("Create Scene...", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("New scene name: ");
                ImGui::SameLine();
                static char namebuffer[128];
                ImGui::InputText("##NewSceneName", namebuffer, 128);
                if(ImGui::Button("Create")) {
                    Scene newScene(namebuffer, new Registry());
                    engine->getProject()->addScene(newScene);
                    engine->setCurrentScene(&engine->getProject()->getScenes().back());
                    showNewScenePopup = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if(ImGui::Button("Cancel")) {
                    showNewScenePopup = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if(ImGui::BeginPopupModal("Load Scene...", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Scene");
                ImGui::SameLine();
                static int selected;
                std::vector<const char *> sceneNames;
                sceneNames.reserve(engine->getProject()->getScenes().size());
                for (auto const &scene : engine->getProject()->getScenes()) {
                    sceneNames.push_back(scene.getName().c_str());
                }
                ImGui::Combo("##LoadScene", &selected, sceneNames.data(), sceneNames.size(), 10);
                if (ImGui::Button("Load")) {
                    engine->setCurrentScene(&engine->getProject()->getScenes().at(selected));
                    ImGui::CloseCurrentPopup();
                    showLoadScenePopup = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    showLoadScenePopup = false;
                }
                ImGui::End();
            }

            ImGuiID dockspace_id = ImGui::GetID("mainspace");

            if(gui_init == 0) {
                ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_id); // Add empty node

                ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
                ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.30f, NULL, &dock_main_id);
                ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
                ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.30f, NULL, &dock_main_id);
                ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.70f, NULL, &dock_main_id);

                ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
                ImGui::DockBuilderDockWindow("Assets", dock_id_bottom);
                ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
                ImGui::DockBuilderDockWindow("Viewport", dock_id_up);
                ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
                ImGui::DockBuilderDockWindow("Scene Parameters", dock_id_right);
                ImGui::DockBuilderFinish(dockspace_id);
                gui_init = 1;
            }
            ImGui::DockSpace(dockspace_id);

            //assetPane.render();

            //hierarchyPane.render();

            ImGui::Begin("Viewport");
            {
                bool pushed = false;
                if(guizmoOperationMode == ImGuizmo::TRANSLATE) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                    pushed = true;
                }
                if (ImGui::Button("T")) {
                    guizmoOperationMode = ImGuizmo::TRANSLATE;
                }
                if(pushed)
                    ImGui::PopStyleColor();

                pushed = false;
                ImGui::SameLine(0, 0);

                if(guizmoOperationMode == ImGuizmo::ROTATE) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                    pushed = true;
                }
                if (ImGui::Button("R")) {
                    guizmoOperationMode = ImGuizmo::ROTATE;
                }
                if(pushed)
                    ImGui::PopStyleColor();

                pushed = false;
                ImGui::SameLine(0, 0);

                if(guizmoOperationMode == ImGuizmo::SCALE) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                    pushed = true;
                }
                if (ImGui::Button("S")) {
                    guizmoOperationMode = ImGuizmo::SCALE;
                }
                if(pushed)
                    ImGui::PopStyleColor();

                if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab))) {
                    if(guizmoOperationMode == ImGuizmo::TRANSLATE) {
                        guizmoOperationMode = ImGuizmo::ROTATE;
                    } else if(guizmoOperationMode == ImGuizmo::ROTATE) {
                        guizmoOperationMode = ImGuizmo::SCALE;
                    } else if(guizmoOperationMode == ImGuizmo::SCALE) {
                        guizmoOperationMode = ImGuizmo::TRANSLATE;
                    }
                }

                ImVec2 wpos = ImGui::GetCursorScreenPos();
                ImVec2 wsize = ImGui::GetWindowContentRegionMax();
                wsize.x -= ImGui::GetCursorPosX();
                wsize.y -= ImGui::GetCursorPosY();
                //ImGui::Image(engine->getInternalFB()->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
                sceneViewportWidth = wsize.x;
                sceneViewportHeight = wsize.y;

                ImGuizmo::SetRect(wpos.x, wpos.y, wsize.x, wsize.y);

                ImGuizmo::Enable(true);
                if (engine->getSelected() != nullptr) {
                    auto e = Eigen::Matrix4f();
                    e.setZero();
                    //ImGuizmo::Manipulate(engine->getCamera()->lookThrough().data(),
                    //                     engine->getCamera()->getProjection().data(), guizmoOperationMode,
                    //                     ImGuizmo::WORLD,
                    //                     engine->getSelected()->getComponent<TransformComponent>()->getTransformation().data(),
                    //                     e.data());
                    auto deltaT = Eigen::Vector3f();
                    auto deltaR = Eigen::Vector3f();
                    auto deltaS = Eigen::Vector3f();
                    deltaT.setZero();
                    deltaR.setZero();
                    deltaS.setZero();

                    ImGuizmo::DecomposeMatrixToComponents(e.data(), deltaT.data(), deltaR.data(), deltaS.data());
                    if (guizmoOperationMode == ImGuizmo::TRANSLATE) {
                        //*engine->getSelected()->getComponent<TransformComponent>()->getPosition() += deltaT;
                    } else if (guizmoOperationMode == ImGuizmo::ROTATE) {
                        //*engine->getSelected()->getComponent<TransformComponent>()->getRotation() += deltaR;
                    } else {
                        //*engine->getSelected()->getComponent<TransformComponent>()->getScale() += (deltaS - Eigen::Vector3f(1, 1,1));
                    }
                }
                auto drag = ImGui::GetMouseDragDelta(0);
                if (!ImGuizmo::IsUsing()) {
                    if(ImGui::IsWindowHovered()) {
                        if(ImGui::IsMouseDragging(0)) {
                            selecting = false;
                            engine->getCamera()->getRotation().x() += drag.y / 6.f;
                            engine->getCamera()->getRotation().y() += drag.x / 6.f;
                            ImGui::ResetMouseDragDelta(0);
                        } else if (selecting && ImGui::IsMouseReleased(0)) {
                            int x = ImGui::GetMousePos().x - wpos.x;
                            int y = ImGui::GetMousePos().y - wpos.y;
                            if (x > 0 && y > 0 && x < wsize.x && y < wsize.y) {
                                auto color = engine->getPickingTextureAt(x, y);
                                int id = color.x() & 0xFF + ((color.y() & 0xFF) << 8) + ((color.z() & 0xFF) << 16);
                                if (id != 0) {
                                    auto picked = engine->getScene()->getRegistry()->getEntities()[id - 1];
                                    //engine->setSelected(picked);
                                }
                            }
                        }
                        if(ImGui::IsMouseDown(0) && !ImGui::IsMouseDragging(0)) {
                            selecting = true;
                        }
                    }
                }
                if (ImGui::IsWindowFocused()) {
                    if (ImGui::IsKeyDown('W')) {
                        engine->getCamera()->forward(CAMERA_DELTA);
                    } else if (ImGui::IsKeyDown('S')) {
                        engine->getCamera()->backward(CAMERA_DELTA);
                    }
                    if (ImGui::IsKeyDown('A')) {
                        engine->getCamera()->left(CAMERA_DELTA);
                    } else if (ImGui::IsKeyDown('D')) {
                        engine->getCamera()->right(CAMERA_DELTA);
                    }
                    if (ImGui::IsKeyDown('Q')) {
                        engine->getCamera()->getPosition().y() += CAMERA_DELTA;
                    } else if (ImGui::IsKeyDown('E')) {
                        engine->getCamera()->getPosition().y() -= CAMERA_DELTA;
                    }
                }
            }
            ImGui::End();

            //inspectorPane.render();
            //sceneParamPane.render();

            ImGui::End();
            ImGui::PopStyleVar();
        } else {
            projectSelectorWindow.render();
        }
    }

    int ICEGUI::getSceneViewportWidth() const {
        return sceneViewportWidth;
    }

    int ICEGUI::getSceneViewportHeight() const {
        return sceneViewportHeight;
    }

    ICEGUI::ICEGUI(void* window): window(window){
        applyStyle();
    }

    void ICEGUI::initialize() {
        Logger::Log(Logger::VERBOSE, "Core", "Creating context...");
        ctx = Context::Create(window);
        ctx->initialize();

        Logger::Log(Logger::INFO, "Core", "Engine starting up...");
        api = RendererAPI::Create();
        api->initialize();
        internalFB = Framebuffer::Create({1280, 720, 1});
        pickingFB = Framebuffer::Create({1280, 720, 1});

        engine = new ICEEngine(window, api, internalFB);
        Logger::Log(Logger::VERBOSE, "Core", "Done");


    }

    void ICEGUI::loop() {
        engine->loop(this);
    }

    void ICEGUI::applyStyle() {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        colors[ImGuiCol_Text]                   = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
        colors[ImGuiCol_Border]                 = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_CheckMark]              = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_Button]                 = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
        colors[ImGuiCol_Header]                 = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_DockingPreview]         = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
        colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

        style->ChildRounding = 4.0f;
        style->FrameBorderSize = 1.0f;
        style->FrameRounding = 2.0f;
        style->GrabMinSize = 7.0f;
        style->PopupRounding = 2.0f;
        style->ScrollbarRounding = 12.0f;
        style->ScrollbarSize = 13.0f;
        style->TabBorderSize = 1.0f;
        style->TabRounding = 0.0f;
        style->WindowRounding = 4.0f;
    }

    void ICEGUI::initializeEditorUI() {
        /*hierarchyPane.initialize();
        inspectorPane.initialize();
        assetPane.initialize();
        sceneParamPane.initialize();*/
    }
}

#ifndef ICE_TEST

using namespace ICE;

static void glfw_error_callback(int error, const char* description)
{
    Logger::Log(Logger::FATAL, "Core", "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{

    std::ifstream f(ICE_CONFIG_FILE);
    if(!f.good()) {
        std::ofstream outfile(ICE_CONFIG_FILE);
        outfile.close();
    }
    f.close();

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
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
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        Logger::Log(Logger::FATAL, "Core", "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    io.Fonts->AddFontFromFileTTF("Assets/Fonts/helvetica.ttf", 14.0f);

    ICEGUI gui = ICEGUI(window);
    gui.initialize();
    gui.loop();
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);


    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    /*engine.getConfig().save();
    if(engine.getProject() != nullptr) {
        engine.getProject()->writeToFile(engine.getCamera());
    }*/
    Logger::Log(Logger::VERBOSE, "Core", "Engine shutting off...");
    return 0;
}
#endif