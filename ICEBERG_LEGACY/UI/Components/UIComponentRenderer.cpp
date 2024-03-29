//
// Created by Thomas Ibanez on 30.11.20.
//

#include <ImGUI/imgui.h>
#include <Logger.h>
#include "UIComponentRenderer.h"
#include <ICEEngine.h>
#include <BufferUtils.h>

namespace ICE {

    void UIComponentRenderer::render(ICE::TransformComponent* cmp) {
        ImGui::Text("Transform Component");
        ImGui::Text("Position:");
        ImGui::PushID("ice_pos");
        //renderVector3f(cmp->getPosition());
        ImGui::PopID();

        ImGui::PushID("ice_rot");
        ImGui::Text("Rotation:");
        //renderVector3f(cmp->getRotation());
        ImGui::PopID();

        ImGui::PushID("ice_sca");
        ImGui::Text("Scale:");
        //renderVector3f(cmp->getScale());
        ImGui::PopID();
    }

    void UIComponentRenderer::renderVector3f(Eigen::Vector3f* vec) {
        ImGui::PushItemWidth(60);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        renderLabel("X", 0x990000FF);
        ImGui::InputFloat("##X", &vec->x());
        ImGui::SameLine();
        renderLabel("Y", 0x9900FF00);
        ImGui::InputFloat("##Y", &vec->y());
        ImGui::SameLine();
        renderLabel("Z", 0x99FF0000);
        ImGui::InputFloat("##Z", &vec->z());
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();
    }

    void UIComponentRenderer::render(RenderComponent* cmp) {

        ImGui::Text("Render Component");
        ImGui::SameLine();
        ImGui::PushID("ice_rendercomponent");
        if(ImGui::Button("x")) {
            //engine->getSelected()->removeComponent<RenderComponent>();
        }
        ImGui::Text("Meshes");
        ImGui::SameLine();

        auto meshNames = std::vector<std::string>(engine->getAssetBank()->getAll<Mesh>().size());
        int i = 0;
        int selected = 0;
        for(const auto& e : engine->getAssetBank()->getAll<Mesh>()) {
            meshNames[i++] = engine->getAssetBank()->getName(e.first).toString();
            //if(e.first == cmp->getMesh()) {
            //    selected = i-1;
            //}
        }
        ImGui::Combo("##Mesh", &selected, BufferUtils::CreateCharBuffer(meshNames).data(), meshNames.size(), 10);
        //cmp->setMesh(engine->getAssetBank()->getUID(std::string(meshNames[selected])));
        ImGui::Text("Material");
        ImGui::SameLine();

        auto materialNames = std::vector<std::string>(engine->getAssetBank()->getAll<Material>().size());
        i = 0;
        selected = 0;
        for(const auto& e : engine->getAssetBank()->getAll<Material>()) {
            materialNames[i++] = engine->getAssetBank()->getName(e.first).toString();
            //if(e.first == cmp->getMaterial()) {
            //    selected = i-1;
            //}
        }
        ImGui::Combo("##Material", &selected, BufferUtils::CreateCharBuffer(materialNames).data(), materialNames.size(), 10);
        //cmp->setMaterial(engine->getAssetBank()->getUID(std::string(materialNames[selected])));
        ImGui::Text("Shader");
        ImGui::SameLine();

        auto shaderNames = std::vector<std::string>(engine->getAssetBank()->getAll<Shader>().size());
        i = 0;
        selected = 0;
        for(const auto& e : engine->getAssetBank()->getAll<Shader>()) {
            shaderNames[i++] = engine->getAssetBank()->getName(e.first).toString();
            //if(e.first == cmp->getShader()) {
            //    selected = i-1;
            //}
        }
        ImGui::Combo("##Shader", &selected, BufferUtils::CreateCharBuffer(shaderNames).data(), shaderNames.size(), 10);
        //cmp->setShader(engine->getAssetBank()->getUID(std::string(shaderNames[selected])));
        ImGui::PopID();

    }

    void UIComponentRenderer::render(LightComponent* lc) {
        ImGui::Text("Light Component");
        ImGui::SameLine();
        ImGui::PushID("ice_lightcomponent");
        if(ImGui::Button("x")) {
            //engine->getSelected()->removeComponent<LightComponent>();
        }
        const char *types[3] = {
                "Point Light",
                "Spot Light",
                "Directional Light"
        };
        int selected = 0;
        /*switch (lc->getType()) {
            case PointLight:
                selected = 0;
                break;
            case SpotLight:
                selected = 1;
                break;
            case DirectionalLight:
                selected = 2;
                break;
        }
        */
        ImGui::Text("Light Type");
        ImGui::SameLine();
        ImGui::Combo("##Light Type", &selected, types, 3, 3);
        /*switch(selected) {
            case 0:
                lc->setType(PointLight);
                break;
            case 1:
                lc->setType(SpotLight);
                break;
            case 2:
                lc->setType(DirectionalLight);
                break;
        }
        ImGui::Text("Color");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(128);
        ImGui::ColorPicker3("##color", lc->getColor().data());
        */
        ImGui::PopID();
    }

    UIComponentRenderer::UIComponentRenderer(ICEEngine *engine) : engine(engine) {}

    void UIComponentRenderer::renderLabel(const char *text, uint32_t backgroundColor) {
        auto dl = ImGui::GetWindowDrawList();
        ImVec2 rectSize = ImGui::CalcTextSize(text);
        rectSize.x = rectSize.y;
        ImVec2 wpos = ImGui::GetWindowPos();
        ImVec2 cpos = ImGui::GetCursorPos();
        ImVec2 pad = ImGui::GetStyle().FramePadding;
        ImVec2 cursor = ImVec2(wpos.x + cpos.x, wpos.y + cpos.y);
        ImVec2 max = ImVec2(cursor.x + rectSize.x + pad.x * 2, cursor.y + rectSize.y + pad.y * 2);
        dl->AddRectFilled(cursor, max, backgroundColor, 0.0f);
        ImGui::SetCursorPosY(cpos.y + pad.y);
        ImGui::SetCursorPosX(cpos.x + pad.x);
        ImGui::Text("%s", text);
        ImGui::SameLine(0, pad.x);
        ImGui::SetCursorPosY(cpos.y);
    }
}
