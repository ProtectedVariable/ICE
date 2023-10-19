// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "ICEEngine.h"
#include <EngineConfig.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.

#include <Logger.h>
#include <ForwardRenderer.h>
#include <OBJLoader.h>
#include <TransformComponent.h>
#include <FileUtils.h>
#include <GLFW/glfw3.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>
#include <ImGUI/imgui_impl_glfw.h>
#include <ImGUI/imgui_impl_opengl3.h>
#include <ImGUI/ImGuizmo.h>
#include <ImGUI/imgui_internal.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif



namespace ICE {
    ICEEngine::ICEEngine(void* window, RendererAPI* api, Framebuffer* framebuffer): systems(std::vector<System*>()),
                                        camera(Camera(CameraParameters{ {60, 16.f / 9.f, 0.1f, 100000 }, Perspective })),
                                        config(EngineConfig::LoadFromFile()), window(window), api(api), internalFB(framebuffer) {
		selected = nullptr;
    }

    void ICEEngine::initialize() {
        Logger::Log(Logger::INFO, "Core", "Engine starting up...");
    }

    void ICEEngine::loop(GUI* gui) {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        while (!glfwWindowShouldClose(static_cast<GLFWwindow*>(window)))
        {
            api->bindDefaultFramebuffer();
            glfwPollEvents();
            api->setClearColor(0,0,0,1);
            api->clear();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGuizmo::BeginFrame();
            ImGuizmo::SetOrthographic(false);

            gui->render();
            // Rendering
            ImGui::Render();


            if(project != nullptr) {
                auto fmt = internalFB->getFormat();
                renderSystem->setTarget(internalFB, fmt.width, fmt.height);
                camera.setParameters({60, (float) fmt.width / (float) fmt.height, 0.01f, 1000});
                api->setClearColor(0,0,0,1);
                api->clear();
                for (auto s : systems) {
                    s->update(currentScene, 0.f);
                }
            }
            int display_w, display_h;
            glfwGetFramebufferSize(static_cast<GLFWwindow *>(window), &display_w, &display_h);
            api->setViewport(0, 0, display_w, display_h);
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

    Camera *ICEEngine::getCamera() {
        return &camera;
    }

    AssetBank* ICEEngine::getAssetBank() {
        return project->getAssetBank();
    }

    Scene *ICEEngine::getScene() {
        return currentScene;
    }

    Entity *ICEEngine::getSelected() const {
        return selected;
    }

    void ICEEngine::setSelected(Entity *selected) {
        ICEEngine::selected = selected;
    }

    Eigen::Vector4i ICEEngine::getPickingTextureAt(int x, int y) {
        /*pickingFB->bind();
        pickingFB->resize(gui.getSceneViewportWidth(), gui.getSceneViewportHeight());
        api->setViewport(0, 0, gui.getSceneViewportWidth(), gui.getSceneViewportHeight());
        camera.setParameters(
                {60, (float) gui.getSceneViewportWidth() / (float) gui.getSceneViewportHeight(), 0.01f, 1000});
        api->setClearColor(0,0,0,0);
        api->clear();
        getAssetBank()->getAsset<Shader>("__ice__picking_shader")->bind();
        getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("projection", camera.getProjection());
        getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("view", camera.lookThrough());
        int id = 1;
        for(auto e : currentScene->getRegistry()->getEntities()) {
            //getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("model", e->getComponent<TransformComponent>()->getTransformation());
            //getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadInt("objectID", id++);
            //if(e->hasComponent<RenderComponent>()) {
            //    api->renderVertexArray(getAssetBank()->getAsset<Mesh>(e->getComponent<RenderComponent>()->getMesh())->getVertexArray());
            //}
        }
        auto color = internalFB->readPixel(x, y);
        internalFB->unbind();
        return color;
        */
       return Eigen::Vector4i();
    }

    RendererAPI *ICEEngine::getApi() const {
        return api;
    }

    Project *ICEEngine::getProject() const {
        return project;
    }

    void ICEEngine::setProject(Project *project) {
        this->project = project;
        this->currentScene = &project->getScenes().at(0);
        this->camera.getPosition() = project->getCameraPosition();
        this->camera.getRotation() = project->getCameraRotation();
        Renderer* renderer = new ForwardRenderer();
        renderer->initialize(RendererConfig(), project->getAssetBank());
        this->renderSystem = new RenderSystem();
        systems.push_back(renderSystem);
        Skybox::Initialize();
        //this->gui.initializeEditorUI();
    }

    EngineConfig &ICEEngine::getConfig() {
        return config;
    }

    int import_cnt = 0;
    void ICEEngine::importMesh() {
        const std::string file = FileUtils::openFileDialog("obj");
        if(file != "") {
            std::string aname = "imported_mesh_"+std::to_string(import_cnt++);
            getAssetBank()->addResource<Mesh>(aname, {file});
            project->copyAssetFile("Meshes", aname, file);
        }
    }

    void ICEEngine::importTexture(bool cubeMap) {
        const std::string file = FileUtils::openFileDialog("");
        if(file != "") {
            std::string aname = "imported_texture_"+std::to_string(import_cnt++);
            if(cubeMap) {
                getAssetBank()->addResource<TextureCube>(aname, {file});
            } else {
                getAssetBank()->addResource<Texture2D>(aname, {file});
            }
            project->copyAssetFile("Textures", aname, file);
        }
    }

    void ICEEngine::setCurrentScene(Scene *currentScene) {
        ICEEngine::currentScene = currentScene;
    }
}

