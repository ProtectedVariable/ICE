//
// Created by Thomas Ibanez on 25.11.20.
//

#ifndef ICE_ICEENGINE_H
#define ICE_ICEENGINE_H

#include <GL/gl3w.h>
#include <vector>
#include <GUI/ICEGUI.h>
#include <Assets/AssetBank.h>
#include <IO/Project.h>
#include <ECS/System.h>
#include <IO/EngineConfig.h>
#include <ECS/RenderSystem.h>
#include <ECS/Registry.h>

namespace ICE {
    class ICEEngine {
    public:
        ICEEngine(void* window);

        void initialize();
        void loop();
        Eigen::Vector4i getPickingTextureAt(int x, int y);

        Framebuffer *getInternalFB() const;

        Camera *getCamera();

        AssetBank* getAssetBank();

        Scene* getScene();

        Entity *getSelected() const;

        RendererAPI *getApi() const;

        Project *getProject() const;

        void setProject(Project *project);

        void setSelected(Entity *selected);

        EngineConfig &getConfig();

        void importMesh();

        void importTexture(bool cubeMap);

        void setCurrentScene(Scene *currentScene);

    private:
        std::vector<System*> systems;
        void* window;
        Scene* currentScene;
        RendererAPI* api;
        Context* ctx;
        ICEGUI gui;
        Framebuffer* internalFB;
        Framebuffer* pickingFB;
        Camera camera;
        Entity* selected;
        Project* project = nullptr;
        EngineConfig config;
        RenderSystem* renderSystem;
        Registry registry;
    };
}


#endif //ICE_ICEENGINE_H
