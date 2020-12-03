//
// Created by Thomas Ibanez on 27.11.20.
//

#ifndef ICE_ICEGUI_H
#define ICE_ICEGUI_H

#include <Graphics/Framebuffer.h>
#include <Scene/Scene.h>
#include <Core/ICEEngine.h>
#include "ICEPane.h"
#include <ImGUI/imgui.h>
#include <ImGUI/ImGuizmo.h>

namespace ICE {
    class ICEEngine;

    class ICEGUI {
    public:
        ICEGUI(ICEEngine* engine);

        void renderImGui();

        int getSceneViewportWidth() const;

        int getSceneViewportHeight() const;

    private:
        int init = 0;
        int sceneViewportWidth, sceneViewportHeight;
        ICEPane* hierarchyPane, *inspectorPane, *assetPane;
        ICEEngine* engine;
        ImGuizmo::OPERATION guizmoOperationMode = ImGuizmo::OPERATION::TRANSLATE;
    };
}


#endif //ICE_ICEGUI_H
