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

    void loop();

    Eigen::Vector4i getPickingTextureAt(int x, int y);

    Camera* getCamera();

    AssetBank* getAssetBank();

    Scene* getScene();

    Entity* getSelected() const;

    RendererAPI* getApi() const;

    Project* getProject() const;

    void setProject(Project* project);

    void setSelected(Entity* selected);

    EngineConfig& getConfig();

    void importMesh();

    void importTexture(bool cubeMap);

    void setCurrentScene(Scene* currentScene);

   private:
    std::vector<System*> systems;

    Context* ctx;
    RendererAPI* api;
    void* window;
    Framebuffer* internalFB;

    Scene* currentScene;
    Camera camera;
    Entity* selected;
    Project* project = nullptr;
    EngineConfig config;
    RenderSystem* renderSystem;
    Registry registry;
};
}  // namespace ICE

#endif  //ICE_ICEENGINE_H
