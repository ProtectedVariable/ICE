#define STB_IMAGE_IMPLEMENTATION
#include "ICEEngine.h"

#include <EngineConfig.h>
#include <FileUtils.h>
#include <ForwardRenderer.h>
#include <GLFW/glfw3.h>
#include <Logger.h>
#include <OBJLoader.h>
#include <TransformComponent.h>

namespace ICE {
ICEEngine::ICEEngine() : camera(std::make_shared<Camera>(CameraParameters{{60, 16.f / 9.f, 0.1f, 100000}, Perspective})), config(EngineConfig::LoadFromFile()) {
    selected = 0;
}

void ICEEngine::initialize() {
    Logger::Log(Logger::INFO, "Core", "Engine starting up...");
}

void ICEEngine::step() {

    api->bindDefaultFramebuffer();
    glfwPollEvents();
    api->setClearColor(0, 0, 0, 1);
    api->clear();

    auto fmt = internalFB->getFormat();
    //renderSystem->setTarget(internalFB, fmt.width, fmt.height);
    camera->setParameters({60, (float) fmt.width / (float) fmt.height, 0.01f, 1000});
    api->setClearColor(0, 0, 0, 1);
    api->clear();
    for (auto s : systems) {
        //s->update(currentScene, 0.f);
    }

    int display_w, display_h;
    glfwGetFramebufferSize(static_cast<GLFWwindow *>(window), &display_w, &display_h);
    api->setViewport(0, 0, display_w, display_h);

    glfwSwapBuffers(static_cast<GLFWwindow *>(window));
}

std::shared_ptr<Camera> ICEEngine::getCamera() {
    return camera;
}

std::shared_ptr<AssetBank> ICEEngine::getAssetBank() {
    //return project->getAssetBank();
    return nullptr;
}

std::shared_ptr<Scene> ICEEngine::getScene() {
    return currentScene;
}

Entity ICEEngine::getSelected() const {
    return selected;
}

void ICEEngine::setSelected(Entity selected) {
    ICEEngine::selected = selected;
}

Eigen::Vector4i ICEEngine::getPickingTextureAt(int x, int y) {
    /* pickingFB->bind();
    pickingFB->resize(gui.getSceneViewportWidth(), gui.getSceneViewportHeight());
    api->setViewport(0, 0, gui.getSceneViewportWidth(), gui.getSceneViewportHeight());
    camera.setParameters({60, (float) gui.getSceneViewportWidth() / (float) gui.getSceneViewportHeight(), 0.01f, 1000});
    api->setClearColor(0, 0, 0, 0);
    api->clear();
    getAssetBank()->getAsset<Shader>("__ice__picking_shader")->bind();
    getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("projection", camera.getProjection());
    getAssetBank()->getAsset<Shader>("__ice__picking_shader")->loadMat4("view", camera.lookThrough());
    int id = 1;
    for (auto e : currentScene->getRegistry()->getEntities()) {
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

std::shared_ptr<RendererAPI> ICEEngine::getApi() const {
    return api;
}

std::shared_ptr<Project> ICEEngine::getProject() const {
    return project;
}

void ICEEngine::setProject(const std::shared_ptr<Project> &project) {
    this->project = project;
    //this->currentScene = project->getScenes().at(0);
    this->camera->getPosition() = project->getCameraPosition();
    this->camera->getRotation() = project->getCameraRotation();
    std::shared_ptr<Renderer> renderer = std::make_shared<ForwardRenderer>();
    renderer->initialize(RendererConfig(), project->getAssetBank());
    //this->renderSystem = std::make_shared<RenderSystem>();
    //systems.push_back(renderSystem);
    Skybox::Initialize();
    //this->gui.initializeEditorUI();
}

EngineConfig &ICEEngine::getConfig() {
    return config;
}

int import_cnt = 0;
void ICEEngine::importMesh() {
    const std::string file = FileUtils::openFileDialog("obj");
    if (file != "") {
        std::string aname = "imported_mesh_" + std::to_string(import_cnt++);
        getAssetBank()->addAsset<Mesh>(aname, {file});
        project->copyAssetFile("Meshes", aname, file);
    }
}

void ICEEngine::importTexture(bool cubeMap) {
    const std::string file = FileUtils::openFileDialog("");
    if (file != "") {
        std::string aname = "imported_texture_" + std::to_string(import_cnt++);
        if (cubeMap) {
            getAssetBank()->addAsset<TextureCube>(aname, {file});
        } else {
            getAssetBank()->addAsset<Texture2D>(aname, {file});
        }
        project->copyAssetFile("Textures", aname, file);
    }
}

void ICEEngine::setCurrentScene(const std::shared_ptr<Scene> &currentScene) {
    ICEEngine::currentScene = currentScene;
}
}  // namespace ICE
