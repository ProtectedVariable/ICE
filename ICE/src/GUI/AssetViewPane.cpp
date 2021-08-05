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
    bool AssetViewPane::render() {
        static int y = 45;
        ImGui::BeginChild("Asset View");
        char buffer[512];
        AssetPath assetName = engine->getAssetBank()->getName(*selectedAsset);
        AssetPath newName = AssetPath(assetName);
        strcpy(buffer, assetName.getName().c_str());
        ImGui::Text("Asset Name");
        ImGui::SameLine();
        ImGuiInputTextFlags flags = (assetName.getName().find("__ice__") == std::string::npos) ? 0 : ImGuiInputTextFlags_ReadOnly;
        if(ImGui::InputText("##Asset Name", buffer, 512, flags)) {
            newName.setName(buffer);
            if(engine->getProject()->renameAsset(assetName, newName)) {

            }
        }
        ImVec2 wsize = ImGui::GetWindowContentRegionMax();
        ImVec2 pos = ImGui::GetCursorPos();
        wsize = ImVec2(wsize.x - pos.x, wsize.y - pos.y);
        if(engine->getAssetBank()->getAll<Texture>().find(*selectedAsset) == engine->getAssetBank()->getAll<Texture>().end()) {
            AssetUID previewMeshId = engine->getAssetBank()->getAll<Mesh>().find(*selectedAsset) == engine->getAssetBank()->getAll<Mesh>().end() ? engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Mesh>("__ice__sphere")) : *selectedAsset;
            AssetUID mat = engine->getAssetBank()->getAll<Material>().find(*selectedAsset) == engine->getAssetBank()->getAll<Material>().end() ? engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Material>("__ice__base_material")) :  *selectedAsset;

            auto scene = Scene("__ice__assetview_scene");

            auto sphere = Entity();
            auto rcSphere = RenderComponent(previewMeshId, mat, engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Shader>("__ice__phong_shader")));

            Mesh* previewMesh = engine->getAssetBank()->getAsset<Mesh>(previewMeshId);
            if(previewMesh != nullptr) {

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
                renderer.setClearColor(Eigen::Vector4f(0, 0, 0.2, 1));

                renderer.submitScene(&scene);
                renderer.prepareFrame(camera);
                renderer.resize(wsize.x, wsize.y);

                tcSphere.getRotation()->y() = y++;
                renderer.render();
                renderer.endFrame();

                ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
            }
        } else if(engine->getAssetBank()->getAll<Texture>().find(*selectedAsset) != engine->getAssetBank()->getAll<Texture>().end()) {
            Texture* tex = engine->getAssetBank()->getAsset<Texture>(*selectedAsset);
            if(tex->getType() == TextureType::Tex2D) {
                ImGui::Image(tex->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
            } else if(tex->getType() == TextureType::CubeMap) {
                Scene scene("__ice__assetview_scene");
                scene.setSkybox(*selectedAsset);
                renderer.setTarget(viewFB);
                renderer.submitScene(&scene);
                renderer.resize(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE);
                Camera camera = Camera({{30, 1.f, 0.01f, 1000 }, Perspective});
                camera.getRotation().y() = y++ /2;
                renderer.prepareFrame(camera);
                renderer.render();
                renderer.endFrame();

                ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
            }
        }
        ImGui::EndChild();
        return true;
    }

    AssetViewPane::AssetViewPane(ICEEngine *engine, AssetUID* selectedAsset) : engine(engine), selectedAsset(selectedAsset),
                                                                                    viewFB(Framebuffer::Create({400, 400, 1})),
                                                                                    camera(Camera({{60, 16.f / 9.f, 0.01f, 1000 }, Perspective })),
                                                                                    renderer(ForwardRenderer()) {
        camera.getPosition().y() = 1;
        camera.getPosition().z() = 2;
        camera.getRotation().x() = -30;
    }

    void AssetViewPane::initialize() {
        renderer.initialize(RendererConfig(), engine->getAssetBank());
    }
}
