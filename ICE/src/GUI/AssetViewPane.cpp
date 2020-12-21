//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetViewPane.h"
#include <ImGUI/imgui.h>
#include <Util/ICEMath.h>
#include <Graphics/Shader.h>
#include <Core/ICEEngine.h>
#include <cstring>

namespace ICE {
    int y = 45;
    bool AssetViewPane::render() {
        ImGui::Begin("Asset View");
        char buffer[512];
        strcpy(buffer, selectedAsset->c_str());
        ImGui::Text("Asset Name");
        ImGui::SameLine();
        ImGuiInputTextFlags flags = (selectedAsset->find("__ice__") == std::string::npos) ? 0 : ImGuiInputTextFlags_ReadOnly;
        if(ImGui::InputText("##Asset Name", buffer, 512, flags)) {
            if(engine->getAssetBank()->renameAsset(*selectedAsset, buffer)) {
                *selectedAsset = buffer;
            }
        }
        Mesh* previewMesh = engine->getAssetBank()->getMeshes().find(*selectedAsset) == engine->getAssetBank()->getMeshes().end() ? engine->getAssetBank()->getMesh("__ice__sphere") :  engine->getAssetBank()->getMesh(*selectedAsset);
        Material* mat = engine->getAssetBank()->getMaterials().find(*selectedAsset) == engine->getAssetBank()->getMaterials().end() ? engine->getAssetBank()->getMaterial("__ice__base_material") :  engine->getAssetBank()->getMaterial(*selectedAsset);
        ImVec2 wsize = ImGui::GetWindowContentRegionMax();
        ImVec2 pos = ImGui::GetCursorPos();
        wsize = ImVec2(wsize.x - pos.x, wsize.y - pos.y);
        viewFB->bind();
        viewFB->resize(wsize.x, wsize.y);
        engine->getApi()->setViewport(0, 0, wsize.x, wsize.y);
        engine->getApi()->clear();
        Shader* shader = engine->getAssetBank()->getShader("__ice__phong_shader");
        shader->bind();
        camera.setParameters({60, wsize.x / wsize.y, 0.01f, 1000});
        shader->loadMat4("projection", camera.getProjection());
        shader->loadMat4("view", camera.lookThrough());
        shader->loadMat4("model", rotationMatrix(Eigen::Vector3f(0, y++, 0)));
        shader->loadFloat3("lights[0].position", Eigen::Vector3f(10,20,10));
        shader->loadFloat3("lights[0].color", Eigen::Vector3f(1,1,1));
        shader->loadInt("light_count", 1);
        shader->loadFloat3("ambient_light", Eigen::Vector3f(0.3,0.3,0.3));
        shader->loadFloat3("material.albedo", mat->getAlbedo());
        shader->loadFloat3("material.specular", mat->getSpecular());
        shader->loadFloat3("material.ambient", mat->getAmbient());
        shader->loadFloat("material.alpha", mat->getAlpha());
        engine->getApi()->renderVertexArray(previewMesh->getVertexArray());
        viewFB->unbind();
        ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        return true;
    }

    AssetViewPane::AssetViewPane(ICEEngine *engine, std::string* selectedAsset) : engine(engine), selectedAsset(selectedAsset),
                                                                                    viewFB(Framebuffer::Create({400, 400, 1})),
                                                                                    camera(Camera({{60, 16.f / 9.f, 0.01f, 1000 }, Perspective })) {
        camera.getPosition().y() = 1;
        camera.getPosition().z() = 2;
        camera.getRotation().x() = -30;
    }
}
