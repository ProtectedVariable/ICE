// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#define TINYOBJLOADER_IMPLEMENTATION
#include "ICEEngine.h"
#include <GLFW/glfw3.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_glfw.h>
#include <ImGUI/imgui_impl_opengl3.h>
#include <ImGUI/ImGuizmo.h>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include <Util/Logger.h>
#include <Graphics/ForwardRenderer.h>
#include <Util/OBJLoader.h>
#include <Scene/TransformComponent.h>
#include <Graphics/RenderSystem.h>
#include <ImGUI/imgui_internal.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif



namespace ICE {
    ICEEngine::ICEEngine(void* window): systems(std::vector<System*>()), window(window) {
        api = RendererAPI::Create();
    }

    void ICEEngine::initialize() {
        Logger::Log(Logger::INFO, "Core", "Engine starting up...");
        Logger::Log(Logger::VERBOSE, "Core", "Creating context...");
        ctx = Context::Create(window);
        ctx->initialize();
        Logger::Log(Logger::VERBOSE, "Core", "Done");

        api->initialize();
        Renderer* renderer = new ForwardRenderer();
        renderer->initialize(api, RendererConfig());
        api->setViewport(0, 0, 1280, 720);

        this->assetBank = new AssetBank();

        this->currentScene = new Scene();
        camera = new Camera(CameraParameters{ {60, 16.f / 9.f, 0.01f, 1000 }, Perspective } );
        camera->getPosition().z() = 1;
    /*
        Entity* bunny = new Entity();
        Mesh* mesh = OBJLoader::loadFromOBJ("Assets/bunny.obj");
        Shader* shader = Shader::Create("Assets/test.vs","Assets/test.fs");
        Material* mat = new Material(shader);
        RenderComponent* rc = new RenderComponent(mesh, mat);
        bunny->addComponent(rc);
        auto tc = new TransformComponent();
        bunny->addComponent(tc);

        currentScene->addEntity("root", "bunny", bunny);
*/
        systems.push_back(new RenderSystem(renderer, camera));

        internalFB = Framebuffer::Create({1280, 720, 1});
        pickingFB = Framebuffer::Create({1280, 720, 1});

        gui = new ICEGUI(this);
    }

    void ICEEngine::loop() {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        while (!glfwWindowShouldClose(static_cast<GLFWwindow*>(window)))
        {
            glfwPollEvents();
            api->clear();

            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGuizmo::BeginFrame();
            ImGuizmo::SetOrthographic(false);

            gui->renderImGui();
            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(static_cast<GLFWwindow *>(window), &display_w, &display_h);
            api->clear();
            internalFB->bind();
            internalFB->resize(gui->getSceneViewportWidth(), gui->getSceneViewportHeight());
            glViewport(0, 0, gui->getSceneViewportWidth(), gui->getSceneViewportHeight());
            camera->setParameters({60, (float) gui->getSceneViewportWidth() / (float) gui->getSceneViewportHeight(), 0.01f, 1000},Perspective);
            api->clear();
            for(auto s : systems) {
                s->update(currentScene,0.f);
            }

            internalFB->unbind();

            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
            glfwSwapBuffers(static_cast<GLFWwindow*>(window));
        }
    }

    Framebuffer *ICEEngine::getInternalFB() const {
        return internalFB;
    }

    Camera *ICEEngine::getCamera() const {
        return camera;
    }

    AssetBank *ICEEngine::getAssetBank() const {
        return assetBank;
    }

    Scene *ICEEngine::getScene() const {
        return currentScene;
    }

    Entity *ICEEngine::getSelected() const {
        return selected;
    }

    void ICEEngine::setSelected(Entity *selected) {
        ICEEngine::selected = selected;
    }

    Eigen::Vector4i ICEEngine::getPickingTextureAt(int x, int y) {
        pickingFB->bind();
        pickingFB->resize(gui->getSceneViewportWidth(), gui->getSceneViewportHeight());
        glViewport(0, 0, gui->getSceneViewportWidth(), gui->getSceneViewportHeight());
        camera->setParameters({60, (float) gui->getSceneViewportWidth() / (float) gui->getSceneViewportHeight(), 0.01f, 1000},Perspective);
        api->clear();
        assetBank->getShader("__ice__picking_shader__")->bind();
        assetBank->getShader("__ice__picking_shader__")->loadMat4("projection", camera->getProjection());
        assetBank->getShader("__ice__picking_shader__")->loadMat4("view", camera->lookThrough());
        int id = 1;
        for(auto e : currentScene->getEntities()) {
            assetBank->getShader("__ice__picking_shader__")->loadMat4("model", e->getComponent<TransformComponent>()->getTransformation());
            assetBank->getShader("__ice__picking_shader__")->loadInt("objectID", id++);
            if(e->hasComponent<RenderComponent>()) {
                api->renderVertexArray(e->getComponent<RenderComponent>()->getMesh()->getVertexArray());
            }
        }
        auto color = internalFB->readPixel(x, y);
        internalFB->unbind();
        return color;
    }
}

using namespace ICE;

static void glfw_error_callback(int error, const char* description)
{
    Logger::Log(Logger::FATAL, "Core", "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
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

    ICEEngine engine = ICEEngine(window);
    engine.initialize();
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
    io.Fonts->AddFontFromFileTTF("Assets/helvetica.ttf", 13.0f);

    engine.loop();
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

    return 0;
}
