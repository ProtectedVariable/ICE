#pragma once
#include <ForwardRenderer.h>
#include <Material.h>
#include <PerspectiveCamera.h>
#include <TransformComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <imgui.h>

#include <tuple>
#include <vector>

#include "Components/ComboBox.h"
#include "Components/InputText.h"
#include "Components/UniformInputs.h"
#include "Dialog.h"

class ShaderEditDialog : public Dialog, ImXML::XMLEventHandler {
   public:
    ShaderEditDialog() : m_xml_tree(ImXML::XMLReader().read("XML/MaterialEditPopup.xml")) {
        m_xml_renderer.addDynamicBind("str_shader_name", {m_shader_name, 512, ImXML::Chars});
    }

    std::string getName() const { return std::string(m_shader_name); }

    void render() override {
        ImGui::PushID(m_dialog_id);
        if (isOpenRequested())
            ImGui::OpenPopup("Material Editor");
        //m_xml_renderer.render(m_xml_tree, *this);
        if (ImGui::BeginTabBar("test", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
            if (ImGui::BeginTabItem("Vertex Stage")) {
                ImGui::Text("Vertex Shader Code Editor Here");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Fragment Stage")) {
                ImGui::Text("Fragment Shader Code Editor Here");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::PopID();
    }

    void onNodeBegin(ImXML::XMLNode& node) override {}
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_apply") {
            ImGui::CloseCurrentPopup();
            done(DialogResult::Ok);
        } else if (node.arg<std::string>("id") == "btn_cancel") {
            ImGui::CloseCurrentPopup();
            done(DialogResult::Cancel);
        }
    }

   private:
    char m_shader_name[512] = {0};

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
