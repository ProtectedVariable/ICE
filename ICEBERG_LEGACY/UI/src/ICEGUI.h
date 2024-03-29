//
// Created by Thomas Ibanez on 27.11.20.
//

#ifndef ICE_ICEGUI_H
#define ICE_ICEGUI_H

#include <Framebuffer.h>
#include <ICEEngine.h>
#include <ImGUI/ImGuizmo.h>
#include <ImGUI/imgui.h>
#include <Scene.h>

#include "AssetPane.h"
#include "HierarchyPane.h"
#include "ICEPane.h"
#include "InspectorPane.h"
#include "ProjectSelectorWindow.h"
#include "SceneParamPane.h"

namespace ICE {
class ICEEngine;

class ICEGUI {
   public:
    ICEGUI(void* window);

    virtual void render();

    int getSceneViewportWidth() const;

    int getSceneViewportHeight() const;

    void initializeEditorUI();

    void initialize();
    void loop();

   private:
    void applyStyle();

    ICEEngine* engine;

    Context* ctx;
    RendererAPI* api;
    Framebuffer* internalFB;
    Framebuffer* pickingFB;

    void* window;
    int sceneViewportWidth, sceneViewportHeight;

    /*HierarchyPane hierarchyPane;
        InspectorPane inspectorPane;
        SceneParamPane sceneParamPane;
        AssetPane assetPane;*/
    ProjectSelectorWindow projectSelectorWindow;
    ImGuizmo::OPERATION guizmoOperationMode = ImGuizmo::OPERATION::TRANSLATE;

    bool showNewScenePopup = false;
    bool showLoadScenePopup = false;

    bool selecting = true;
};
}  // namespace ICE

#endif  //ICE_ICEGUI_H
