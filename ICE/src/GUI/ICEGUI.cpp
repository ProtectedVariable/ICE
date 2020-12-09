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

#define CAMERA_DELTA 0.1f

namespace ICE {
    int cnt = 0;
    int gui_init = 0;

    void ICEGUI::renderImGui() {
        if(engine->getProject() != nullptr) {
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
                    ImGui::MenuItem("New Project");
                    ImGui::MenuItem("Save");
                    ImGui::MenuItem("Load");
                    if(ImGui::BeginMenu("Import...")) {
                        if(ImGui::MenuItem("Mesh (.obj)")) {
                            //TODO: Copy the source file in the project directory, add a link from the asset to the copied source file
                            const std::string file = FileUtils::openFileDialog("obj");
                            if(file != "") {
                                engine->getAssetBank()->addMesh("imported_mesh_"+std::to_string(cnt++), OBJLoader::loadFromOBJ(file));
                            }
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
                        engine->getCamera()->getRotation().x() += drag.y / 300.f;
                        engine->getCamera()->getRotation().y() += drag.x / 300.f;
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

    }
}
