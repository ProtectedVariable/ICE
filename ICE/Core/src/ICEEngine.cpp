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
}

void ICEEngine::initialize(const std::shared_ptr<GraphicsFactory> &graphics_factory, void *window) {
    Logger::Log(Logger::INFO, "Core", "Engine starting up...");

    ctx = graphics_factory->createContext(window);
    api = graphics_factory->createRendererAPI();
    internalFB = graphics_factory->createFramebuffer({512, 512, 1});
}

void ICEEngine::step(const std::shared_ptr<Scene> &scene) {

    auto fmt = internalFB->getFormat();
    m_rendersystem->setTarget(internalFB.get(), fmt.width, fmt.height);
    camera->setParameters({60, (float) fmt.width / (float) fmt.height, 0.01f, 1000});

    for (const auto &s : systems) {
        s->update(scene, 0.f);
    }

}

std::shared_ptr<Camera> ICEEngine::getCamera() {
    return camera;
}

std::shared_ptr<AssetBank> ICEEngine::getAssetBank() {
    return project->getAssetBank();
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

std::shared_ptr<Framebuffer> ICEEngine::getInternalFramebuffer() const {
    return internalFB;
}

void ICEEngine::setProject(const std::shared_ptr<Project> &project) {
    this->project = project;
    this->camera->getPosition() = project->getCameraPosition();
    this->camera->getRotation() = project->getCameraRotation();
    auto renderer = new ForwardRenderer(api.get());
    renderer->initialize(RendererConfig(), project->getAssetBank().get());
    m_rendersystem = std::make_shared<RenderSystem>();
    m_rendersystem->setCamera(camera.get());
    m_rendersystem->setRenderer(renderer);
    auto fmt = internalFB->getFormat();
    m_rendersystem->setTarget(internalFB.get(), fmt.width, fmt.height);
    systems.push_back(m_rendersystem);
    Skybox::Initialize();
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
}  // namespace ICE
