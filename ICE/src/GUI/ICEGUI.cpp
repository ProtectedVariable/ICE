//
// Created by Thomas Ibanez on 27.11.20.
//
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>
#include <Util/Logger.h>
#include <Core/ICEEngine.h>
#include <Platform/FileUtils.h>
#include <Util/OBJLoader.h>
#include "ICEGUI.h"
#include "HierarchyPane.h"
#include "InspectorPane.h"
#include "AssetPane.h"

#define CAMERA_DELTA 1.f

namespace ICE {
    int gui_init = 0;

    void ICEGUI::renderImGui() {
        if(engine->getProject() != nullptr) {

            if(showNewScenePopup) {
                ImGui::Begin("Create New Scene", 0, ImGuiWindowFlags_Modal);
                ImGui::Text("New scene name: ");
                ImGui::SameLine();
                static char namebuffer[128];
                ImGui::InputText("##NewSceneName", namebuffer, 128);
                if(ImGui::Button("Create")) {
                    Scene newScene(namebuffer);
                    engine->getProject()->addScene(newScene);
                    engine->setCurrentScene(&engine->getProject()->getScenes().back());
                    showNewScenePopup = false;
                }
                ImGui::SameLine();
                if(ImGui::Button("Cancel")) {
                    showNewScenePopup = false;
                }
                ImGui::End();
            }

            ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
            flags |= ImGuiWindowFlags_NoDocking;
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Main", 0, flags);
            ImGui::PopStyleVar();

            if (ImGui::BeginMenuBar())
            {
                if(ImGui::BeginMenu("File"))
                {
                    if(ImGui::MenuItem("New Scene")) {
                        showNewScenePopup = true;
                    }
                    ImGui::MenuItem("Load Scene");
                    ImGui::Separator();
                    ImGui::MenuItem("Save Project");
                    ImGui::Separator();
                    if(ImGui::BeginMenu("Import...")) {
                        if(ImGui::MenuItem("Mesh (.obj)")) {
                            engine->importMesh();
                        }
                        if(ImGui::MenuItem("Texture")) {
                            engine->importTexture();
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            ImGuiID dockspace_id = ImGui::GetID("mainspace");

            if(gui_init == 0) {
                ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_id); // Add empty node

                ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
                ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.30f, NULL, &dock_main_id);
                ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
                ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.30f, NULL, &dock_main_id);
                ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.70f, NULL, &dock_main_id);

                ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
                ImGui::DockBuilderDockWindow("Assets", dock_id_bottom);
                ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
                ImGui::DockBuilderDockWindow("Viewport", dock_id_up);
                ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
                ImGui::DockBuilderFinish(dockspace_id);
                gui_init = 1;
            }
            ImGui::DockSpace(dockspace_id);

            assetPane.render();

            hierarchyPane.render();

            ImGui::Begin("Viewport");
            {
                if (ImGui::Button("T")) {
                    guizmoOperationMode = ImGuizmo::TRANSLATE;
                }
                ImGui::SameLine();
                if (ImGui::Button("R")) {
                    guizmoOperationMode = ImGuizmo::ROTATE;
                }
                ImGui::SameLine();
                if (ImGui::Button("S")) {
                    guizmoOperationMode = ImGuizmo::SCALE;
                }
                ImVec2 wpos = ImGui::GetCursorScreenPos();
                ImVec2 wsize = ImGui::GetWindowContentRegionMax();
                wsize.x -= ImGui::GetCursorPosX();
                wsize.y -= ImGui::GetCursorPosY();
                ImGui::Image(engine->getInternalFB()->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
                sceneViewportWidth = wsize.x;
                sceneViewportHeight = wsize.y;

                ImGuizmo::SetRect(wpos.x, wpos.y, wsize.x, wsize.y);

                ImGuizmo::Enable(true);
                if (engine->getSelected() != nullptr) {
                    auto e = Eigen::Matrix4f();
                    e.setZero();
                    ImGuizmo::Manipulate(engine->getCamera()->lookThrough().data(),
                                         engine->getCamera()->getProjection().data(), guizmoOperationMode,
                                         ImGuizmo::WORLD,
                                         engine->getSelected()->getComponent<TransformComponent>()->getTransformation().data(),
                                         e.data());
                    auto deltaT = Eigen::Vector3f();
                    auto deltaR = Eigen::Vector3f();
                    auto deltaS = Eigen::Vector3f();
                    deltaT.setZero();
                    deltaR.setZero();
                    deltaS.setZero();

                    ImGuizmo::DecomposeMatrixToComponents(e.data(), deltaT.data(), deltaR.data(), deltaS.data());
                    if (guizmoOperationMode == ImGuizmo::TRANSLATE) {
                        *engine->getSelected()->getComponent<TransformComponent>()->getPosition() += deltaT;
                    } else if (guizmoOperationMode == ImGuizmo::ROTATE) {
                        *engine->getSelected()->getComponent<TransformComponent>()->getRotation() += deltaR;
                    } else {
                        *engine->getSelected()->getComponent<TransformComponent>()->getScale() += (deltaS - Eigen::Vector3f(1, 1,1));
                    }
                }
                auto drag = ImGui::GetMouseDragDelta();
                if (!ImGuizmo::IsUsing()) {
                    if(ImGui::IsWindowHovered()) {
                        engine->getCamera()->getRotation().x() += drag.y / 100.f;
                        engine->getCamera()->getRotation().y() += drag.x / 100.f;
                    }
                    if (ImGui::IsMouseClicked(0)) {
                        int x = ImGui::GetMousePos().x - wpos.x;
                        int y = ImGui::GetMousePos().y - wpos.y;
                        if (x > 0 && y > 0 && x < wsize.x && y < wsize.y) {
                            auto color = engine->getPickingTextureAt(x, y);
                            int id = color.x() & 0xFF + ((color.y() & 0xFF) << 8) + ((color.z() & 0xFF) << 16);
                            if (id != 0) {
                                auto picked = engine->getScene()->getEntities()[id - 1];
                                engine->setSelected(picked);
                            } else {
                                engine->setSelected(nullptr);
                            }
                        }
                    }
                }
                if (ImGui::IsWindowFocused()) {
                    if (ImGui::IsKeyDown('W')) {
                        engine->getCamera()->forward(CAMERA_DELTA);
                    } else if (ImGui::IsKeyDown('S')) {
                        engine->getCamera()->backward(CAMERA_DELTA);
                    }
                    if (ImGui::IsKeyDown('A')) {
                        engine->getCamera()->left(CAMERA_DELTA);
                    } else if (ImGui::IsKeyDown('D')) {
                        engine->getCamera()->right(CAMERA_DELTA);
                    }
                    if (ImGui::IsKeyDown('Q')) {
                        engine->getCamera()->getPosition().y() += CAMERA_DELTA;
                    } else if (ImGui::IsKeyDown('E')) {
                        engine->getCamera()->getPosition().y() -= CAMERA_DELTA;
                    }
                }
            }
            ImGui::End();

            inspectorPane.render();

            ImGui::End();
            ImGui::PopStyleVar();
        } else {
            projectSelectorWindow.render();
        }
    }

    int ICEGUI::getSceneViewportWidth() const {
        return sceneViewportWidth;
    }

    int ICEGUI::getSceneViewportHeight() const {
        return sceneViewportHeight;
    }

    ICEGUI::ICEGUI(ICEEngine *engine): engine(engine), hierarchyPane(HierarchyPane(engine)), inspectorPane(InspectorPane(engine)), assetPane(AssetPane(engine)), projectSelectorWindow(engine) {
        applyStyle();
    }

    void ICEGUI::applyStyle() {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        colors[ImGuiCol_Text]                   = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
        colors[ImGuiCol_Border]                 = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_CheckMark]              = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_Button]                 = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
        colors[ImGuiCol_Header]                 = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_DockingPreview]         = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
        colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

        style->ChildRounding = 4.0f;
        style->FrameBorderSize = 1.0f;
        style->FrameRounding = 2.0f;
        style->GrabMinSize = 7.0f;
        style->PopupRounding = 2.0f;
        style->ScrollbarRounding = 12.0f;
        style->ScrollbarSize = 13.0f;
        style->TabBorderSize = 1.0f;
        style->TabRounding = 0.0f;
        style->WindowRounding = 4.0f;
    }
}
