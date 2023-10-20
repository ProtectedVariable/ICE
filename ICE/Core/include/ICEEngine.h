//
// Created by Thomas Ibanez on 25.11.20.
//

#ifndef ICE_ICEENGINE_H
#define ICE_ICEENGINE_H

#include <AssetBank.h>
#include <EngineConfig.h>
#include <GL/gl3w.h>
#include <GraphicsAPI.h>
#include <Project.h>
#include <Registry.h>
#include <RenderSystem.h>
#include <System.h>

#include <vector>

namespace ICE {
class ICEEngine {
   public:
    ICEEngine();

    void initialize();

    void step();

    Eigen::Vector4i getPickingTextureAt(int x, int y);

    std::shared_ptr<Camera> getCamera();

    std::shared_ptr<AssetBank> getAssetBank();

    std::shared_ptr<Scene> getScene();

    Entity getSelected() const;

    std::shared_ptr<RendererAPI> getApi() const;

    std::shared_ptr<Project> getProject() const;

    void setProject(const std::shared_ptr<Project>& project);

    void setSelected(Entity selected);

    EngineConfig& getConfig();

    void importMesh();

    void importTexture(bool cubeMap);

    void setCurrentScene(const std::shared_ptr<Scene>& scene);

   private:
    void* window;

    std::vector<std::shared_ptr<System>> systems;

    std::shared_ptr<Context> ctx;
    std::shared_ptr<RendererAPI> api;
    std::shared_ptr<Framebuffer> internalFB;

    std::shared_ptr<Scene> currentScene;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Project> project = nullptr;
    Entity selected;
    EngineConfig config;
    Registry registry;
};
}  // namespace ICE

#endif  //ICE_ICEENGINE_H
