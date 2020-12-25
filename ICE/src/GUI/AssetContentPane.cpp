//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetContentPane.h"
#include <ImGUI/imgui.h>
#include <Util/ICEMath.h>
#include <Core/ICEEngine.h>

namespace ICE {

    bool AssetContentPane::render() {
        if(newMaterialPaneShow) {
            if(!newMaterialPane.render()) {
                newMaterialPaneShow = false;
                newMaterialPane.build();
            }
        }
        ImGui::Begin("Asset Content");
        ImGui::Button("New...");
        if (ImGui::BeginPopupContextItem("add_asset", ImGuiPopupFlags_MouseButtonLeft)) {
            if(ImGui::Button("Mesh")) {
                engine->importMesh();
                ImGui::CloseCurrentPopup();
            }
            if(ImGui::Button("Material")) {
                newMaterialPaneShow = true;
                newMaterialPane.reset();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::Separator();
        ImGui::Columns(6);
        int i = 0;
        if(*selectedDir == 0) {

            for(const auto& m : engine->getAssetBank()->getMeshes()) {
                auto scene = Scene();
                Shader* shader = engine->getAssetBank()->getShader("__ice__normal_shader");
                Material mat = Material(shader);

                auto sphere = Entity();
                auto rcSphere = RenderComponent(&m.second, &mat);
                auto tcSphere = TransformComponent(Eigen::Vector3f(0,0,0), Eigen::Vector3f(0, 45, 0), Eigen::Vector3f(1,1,1));
                sphere.addComponent(&rcSphere);
                sphere.addComponent(&tcSphere);
                scene.addEntity("sphere", &sphere);

                auto light = Entity();
                auto lcLight = LightComponent(PointLight, Eigen::Vector3f(1,1,1));
                auto tcLight = TransformComponent(Eigen::Vector3f(10,20,10), Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1));
                light.addComponent(&lcLight);
                light.addComponent(&tcLight);
                scene.addEntity("light", &light);

                renderer.setTarget(thumbnailFBO[i]);
                renderer.submitScene(&scene);
                renderer.resize(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE);
                renderer.prepareFrame(camera);
                renderer.render();
                renderer.endFrame();

                ImGui::Image(thumbnailFBO[i]->getTexture(), ImVec2(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE), ImVec2(0, 1), ImVec2(1, 0));
                if(ImGui::IsItemClicked(0)) {
                    *selectedAsset = m.first;
                }
                if(*selectedAsset == m.first) {
                    ImGui::TextColored(ImVec4(0,0.8f,1,1), "%s", m.first.c_str());
                } else {
                    ImGui::Text("%s", m.first.c_str());
                }
                ImGui::NextColumn();
                i++;
            }
        } else if(*selectedDir == 1) {
            for(const auto& m : engine->getAssetBank()->getMaterials()) {
                Material mat = m.second;

                auto scene = Scene();

                auto sphere = Entity();
                auto rcSphere = RenderComponent(engine->getAssetBank()->getMesh("__ice__sphere"), &mat);
                auto tcSphere = TransformComponent();
                sphere.addComponent(&rcSphere);
                sphere.addComponent(&tcSphere);
                scene.addEntity("sphere", &sphere);

                auto light = Entity();
                auto lcLight = LightComponent(PointLight, Eigen::Vector3f(1,1,1));
                auto tcLight = TransformComponent(Eigen::Vector3f(10,20,10), Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1));
                light.addComponent(&lcLight);
                light.addComponent(&tcLight);
                scene.addEntity("light", &light);

                renderer.setTarget(thumbnailFBO[i]);
                renderer.submitScene(&scene);
                renderer.resize(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE);
                renderer.prepareFrame(camera);
                renderer.render();
                renderer.endFrame();

                ImGui::Image(thumbnailFBO[i]->getTexture(), ImVec2(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE), ImVec2(0, 1), ImVec2(1, 0));
                if(ImGui::IsItemClicked(0)) {
                    *selectedAsset = m.first;
                }
                if(ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
                    newMaterialPaneShow = true;
                    newMaterialPane.edit(*selectedAsset, mat);
                }
                if(*selectedAsset == m.first) {
                    ImGui::TextColored(ImVec4(0,0.8f,1,1), "%s", m.first.c_str());
                } else {
                    ImGui::Text("%s", m.first.c_str());
                }
                i++;
                ImGui::NextColumn();
            }
        } else if(*selectedDir == 2) {
            for(const auto& m : engine->getAssetBank()->getShaders()) {
                //TODO: Add file icon
                ImGui::Text("%s", m.first.c_str());
                ImGui::NextColumn();
            }
        } else if(*selectedDir == 3) {
            i = 0;
            for(const auto& m : engine->getAssetBank()->getTextures()) {
                ImGui::Image(m.second->getTexture(), ImVec2(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE), ImVec2(0, 1), ImVec2(1, 0));
                if(ImGui::IsItemClicked(0)) {
                    *selectedAsset = m.first;
                }
                if(*selectedAsset == m.first) {
                    ImGui::TextColored(ImVec4(0,0.8f,1,1), "%s", m.first.c_str());
                } else {
                    ImGui::Text("%s", m.first.c_str());
                }
                ImGui::NextColumn();
                i++;
            }
        }
        ImGui::End();
        return true;
    }

    AssetContentPane::AssetContentPane(const int *selectedDir, ICEEngine *engine, std::string* selectedAsset) : selectedDir(selectedDir),
                                                                                    engine(engine), selectedAsset(selectedAsset),
                                                                                    camera(Camera({{30, 1.f, 0.01f, 1000 }, Perspective})),
                                                                                    newMaterialPane(NewMaterialPane(engine)),
                                                                                    renderer(ForwardRenderer()) {
        camera.getPosition().z() = 3;
        camera.getPosition().y() = 2;
        camera.getRotation().x() = -35;
        renderer.initialize(RendererConfig());
        for(int i = 0; i < ICE_MAX_THUMBNAILS; i++) {
            this->thumbnailFBO[i] = Framebuffer::Create({ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE, 1});
        }
    }
}