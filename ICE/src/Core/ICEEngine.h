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
#include <Core/GUI.h>

namespace ICE {
    class ICEEngine {
    public:
        ICEEngine(void* window, RendererAPI* api, Framebuffer* framebuffer);

        void initialize();

        void loop(GUI* gui);

        Eigen::Vector4i getPickingTextureAt(int x, int y);

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
}


#endif //ICE_ICEENGINE_H
