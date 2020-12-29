//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetViewPane.h"
#include <ImGUI/imgui.h>
#include <Math/ICEMath.h>
#include <Graphics/Shader.h>
#include <Core/ICEEngine.h>
#include <cstring>

namespace ICE {
    int y = 45;
    bool AssetViewPane::render() {
        ImGui::BeginChild("Asset View");
        char buffer[512];
        strcpy(buffer, selectedAsset->c_str());
        ImGui::Text("Asset Name");
        ImGui::SameLine();
        ImGuiInputTextFlags flags = (selectedAsset->find("__ice__") == std::string::npos) ? 0 : ImGuiInputTextFlags_ReadOnly;
        if(ImGui::InputText("##Asset Name", buffer, 512, flags)) {
            if(engine->getProject()->renameAsset(*selectedAsset, buffer)) {
                *selectedAsset = buffer;
            }
        }
        ImVec2 wsize = ImGui::GetWindowContentRegionMax();
        ImVec2 pos = ImGui::GetCursorPos();
        wsize = ImVec2(wsize.x - pos.x, wsize.y - pos.y);
        if(engine->getAssetBank()->getTextures().find(*selectedAsset) == engine->getAssetBank()->getTextures().end()) {
            Mesh* previewMesh = engine->getAssetBank()->getMeshes().find(*selectedAsset) == engine->getAssetBank()->getMeshes().end() ? engine->getAssetBank()->getMesh("__ice__sphere") :  engine->getAssetBank()->getMesh(*selectedAsset);
            Material* mat = engine->getAssetBank()->getMaterials().find(*selectedAsset) == engine->getAssetBank()->getMaterials().end() ? engine->getAssetBank()->getMaterial("__ice__base_material") :  engine->getAssetBank()->getMaterial(*selectedAsset);

            auto scene = Scene("__ice__assetview_scene");

            auto sphere = Entity();
            auto rcSphere = RenderComponent(previewMesh, mat, engine->getAssetBank()->getShader("__ice__phong_shader"));
            float scale = 2.f/(previewMesh->getBoundingBox().getMax() - previewMesh->getBoundingBox().getMin()).norm();
            auto tcSphere = TransformComponent();
            tcSphere.getScale()->x() = scale;
            tcSphere.getScale()->y() = scale;
            tcSphere.getScale()->z() = scale;
            sphere.addComponent(&rcSphere);
            sphere.addComponent(&tcSphere);
            scene.addEntity("sphere", &sphere);

            auto light = Entity();
            auto lcLight = LightComponent(PointLight, Eigen::Vector3f(1,1,1));
            auto tcLight = TransformComponent(Eigen::Vector3f(10,20,10), Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1));
            light.addComponent(&lcLight);
            light.addComponent(&tcLight);
            scene.addEntity("light", &light);

            camera.setParameters({60, wsize.x/wsize.y, 0.01f, 100});

            renderer.setTarget(viewFB);
            renderer.submitScene(&scene);
            renderer.prepareFrame(camera);
            renderer.resize(wsize.x, wsize.y);

            tcSphere.getRotation()->y() += y++;
            renderer.render();
            renderer.endFrame();

            ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        } else {
            ImGui::Image(engine->getAssetBank()->getTexture(*selectedAsset)->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::EndChild();
        return true;
    }

    AssetViewPane::AssetViewPane(ICEEngine *engine, std::string* selectedAsset) : engine(engine), selectedAsset(selectedAsset),
                                                                                    viewFB(Framebuffer::Create({400, 400, 1})),
                                                                                    camera(Camera({{60, 16.f / 9.f, 0.01f, 1000 }, Perspective })),
                                                                                    renderer(ForwardRenderer()) {
        camera.getPosition().y() = 1;
        camera.getPosition().z() = 2;
        camera.getRotation().x() = -30;
        renderer.initialize(RendererConfig());
    }
}
