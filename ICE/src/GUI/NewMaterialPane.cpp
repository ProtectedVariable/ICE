//
// Created by Thomas Ibanez on 21.12.20.
//

#include "NewMaterialPane.h"
#include <Core/ICEEngine.h>
#include <Util/ICEMath.h>

#define ICE_NEWMAT_PICKER_WIDTH 150

namespace ICE {
    bool NewMaterialPane::render() {
        auto ret = true;
        ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoOptions;
        ImGui::Begin("Create New Material");
        ImGui::Text("Name");
        ImGui::SameLine();
        char buffer[512];
        strcpy(buffer, name.c_str());
        ImGui::InputText("##Name", buffer, 512);
        name = std::string(buffer);
        ImGui::Text("Albedo");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
        ImGui::ColorPicker3("##NewMaterialAlbedo", albedo.data(), flags);
        ImGui::SameLine();
        ImGui::Text("Specular");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
        ImGui::ColorPicker3("##NewMaterialSpecular", specular.data(), flags);
        ImGui::SameLine();
        ImGui::Text("Ambient");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
        ImGui::ColorPicker3("##NewMaterialAmbient", ambient.data(), flags);
        ImGui::Text("Alpha");
        ImGui::SameLine();
        ImGui::SliderFloat("##NewMaterialAlpha", &alpha, 0, 100);
        ImVec2 wsize = ImGui::GetWindowContentRegionMax();
        ImVec2 pos = ImGui::GetCursorPos();
        wsize = ImVec2(wsize.x - pos.x, wsize.y - pos.y - 30);
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
        shader->loadFloat3("material.albedo", albedo);
        shader->loadFloat3("material.specular", specular);
        shader->loadFloat3("material.ambient",ambient);
        shader->loadFloat("material.alpha", alpha);
        engine->getApi()->renderVertexArray(engine->getAssetBank()->getMesh("__ice__sphere")->getVertexArray());
        viewFB->unbind();
        ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        if(ImGui::Button("Add")) {
            ret = false;
        }
        ImGui::End();
        return ret;
    }

    NewMaterialPane::NewMaterialPane(ICEEngine* engine): engine(engine), name("new_material"),
                                                         viewFB(Framebuffer::Create({200, 200, 1})),
                                                         camera(Camera({{60, 16.f / 9.f, 0.01f, 1000 }, Perspective })) {
        reset();
        camera.getPosition().y() = 1;
        camera.getPosition().z() = 2;
        camera.getRotation().x() = -30;
    }

    void NewMaterialPane::reset() {
        albedo = Eigen::Vector3f(1,1,1);
        specular = Eigen::Vector3f(1,1,1);
        ambient = Eigen::Vector3f(.1f,.1f,.1f);
        alpha = 1.f;
    }

    void NewMaterialPane::build() {
        auto mtl = new Material(engine->getAssetBank()->getShader("__ice__phong_shader"), albedo, specular, ambient, alpha);
        engine->getAssetBank()->addMaterial(name, mtl);
    }
}