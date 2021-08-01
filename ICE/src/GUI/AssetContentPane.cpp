//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetContentPane.h"
#include <ImGUI/imgui.h>
#include <Math/ICEMath.h>
#include <Core/ICEEngine.h>

namespace ICE {

    bool AssetContentPane::render() {
        if(newMaterialPaneShow) {
            if(!newMaterialPane.render()) {
                newMaterialPaneShow = false;
                newMaterialPane.build();
            }
        }
        ImGui::BeginChild("Asset Content");
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

        int i = 0;
        if(*selectedDir == 0) {

            for(const auto& m : engine->getAssetBank()->getAll<Mesh>()) {
                auto scene = Scene("__ice__assetcontent_scene");
                AssetUID shader = engine->getAssetBank()->getUID<Shader>("__ice__normal_shader");
                AssetUID mat = engine->getAssetBank()->getUID<Material>("__ice__base_material");

                auto mesh = Entity();
                auto rcMesh = RenderComponent(m.first, mat, shader);
                float scale = 1.5f/(m.second->getBoundingBox().getMax() - m.second->getBoundingBox().getMin()).norm();
                auto tcMesh = TransformComponent(Eigen::Vector3f(0,0,0), Eigen::Vector3f(0, 45, 0), Eigen::Vector3f(scale, scale, scale));
                mesh.addComponent(&rcMesh);
                mesh.addComponent(&tcMesh);
                scene.addEntity("mesh", &mesh);

                renderer.setTarget(thumbnailFBO[i]);
                renderer.submitScene(&scene);
                renderer.resize(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE);
                renderer.prepareFrame(camera);
                renderer.render();
                renderer.endFrame();
                std::string name = engine->getAssetBank()->getName(m.first);

                renderAssetThumbnail(thumbnailFBO[i]->getTexture(), name.substr(name.find_last_of('/')+1));
                i++;
            }
        } else if(*selectedDir == 1) {
            for(const auto& m : engine->getAssetBank()->getAll<Material>()) {
                Material mat = *m.second;

                auto scene = Scene("__ice__assetcontent_scene");

                auto sphere = Entity();
                auto rcSphere = RenderComponent(engine->getAssetBank()->getUID<Mesh>("__ice__sphere"), m.first, engine->getAssetBank()->getUID<Shader>("__ice__phong_shader"));
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

                std::string name = engine->getAssetBank()->getName(m.first);

                renderAssetThumbnail(thumbnailFBO[i]->getTexture(), name.substr(name.find_last_of('/')+1));                if(ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
                    newMaterialPaneShow = true;
                    newMaterialPane.edit(*selectedAsset, mat);
                }

                i++;
            }
        } else if(*selectedDir == 2) {
            for(const auto& m : engine->getAssetBank()->getAll<Shader>()) {
                //TODO: Add file icon
                ImGui::Text("%s", engine->getAssetBank()->getName(m.first).c_str());
            }
        } else if(*selectedDir == 3) {
            i = 0;
            for(const auto& m : engine->getAssetBank()->getAll<Texture>()) {
                if(m.second->getType() == TextureType::Tex2D) {
                    renderAssetThumbnail(m.second->getTexture(), engine->getAssetBank()->getName(m.first));
                } else if(m.second->getType() == TextureType::CubeMap) {
                    Scene scene("__ice__assetcontent_scene");
                    scene.setSkybox(Skybox(m.first));
                    renderer.setTarget(thumbnailFBO[i]);
                    renderer.submitScene(&scene);
                    renderer.resize(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE);
                    Camera camera = Camera({{30, 1.f, 0.01f, 1000 }, Perspective});
                    renderer.prepareFrame(camera);
                    renderer.render();
                    renderer.endFrame();

                    std::string name = engine->getAssetBank()->getName(m.first);

                    renderAssetThumbnail(thumbnailFBO[i]->getTexture(), name.substr(name.find_last_of('/')+1));
                }
                i++;
            }
        }
        ImGui::EndChild();
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
    }

    void AssetContentPane::renderAssetThumbnail(void *tex, const std::string& name) {
        ImGui::BeginGroup();
        ImGui::Image(tex, ImVec2(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE), ImVec2(0, 1), ImVec2(1, 0));
        if(ImGui::IsItemClicked(0)) {
            *selectedAsset = name;
        }
        if(*selectedAsset == name) {
            ImGui::TextColored(ImVec4(0,0.8f,1,1), "%s", name.c_str());
        } else {
            ImGui::Text("%s", name.c_str());
        }
        ImGui::EndGroup();
        if(ImGui::GetCursorPosX() + ICE_THUMBNAIL_SIZE < ImGui::GetContentRegionAvailWidth()) {
            ImGui::SameLine();
        }
    }

    void AssetContentPane::initialize() {
        renderer.initialize(RendererConfig(), engine->getAssetBank());
        for(int i = 0; i < ICE_MAX_THUMBNAILS; i++) {
            this->thumbnailFBO[i] = Framebuffer::Create({ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE, 1});
        }
    }
}