//
// Created by Thomas Ibanez on 27.11.20.
//

#ifndef ICE_ICEGUI_H
#define ICE_ICEGUI_H

#include <Graphics/Framebuffer.h>
#include <Scene/Scene.h>
#include <Core/ICEEngine.h>
#include "ICEPane.h"
#include "HierarchyPane.h"
#include "InspectorPane.h"
#include "AssetPane.h"
#include "ProjectSelectorWindow.h"
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
        int sceneViewportWidth, sceneViewportHeight;
        HierarchyPane hierarchyPane;
        InspectorPane inspectorPane;
        AssetPane assetPane;
        ProjectSelectorWindow projectSelectorWindow;
        ICEEngine* engine;
        ImGuizmo::OPERATION guizmoOperationMode = ImGuizmo::OPERATION::TRANSLATE;
    };
}


#endif //ICE_ICEGUI_H
