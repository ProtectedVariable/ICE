#pragma once
#include <XMLDynamicBind.h>
#include <XMLEventHandler.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <dialog.h>
#include <imgui.h>

#include "Dialog.h"

class NewProjectPopup : public Dialog, ImXML::XMLEventHandler {
   public:
    NewProjectPopup() : m_xml_tree(ImXML::XMLReader().read("XML/NewProjectPopup.xml")) {
        m_xml_renderer.addDynamicBind("str_project_name", {m_project_name, 512, ImXML::Chars});
        m_xml_renderer.addDynamicBind("str_project_directory", {m_project_dir, 512, ImXML::Chars});
    }

    void render() override {
        if (isOpenRequested())
            ImGui::OpenPopup("New Project");
        m_xml_renderer.render(m_xml_tree, *this);
    }

    void onNodeBegin(ImXML::XMLNode& node) override {}
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_create") {
            auto path_string = std::string(m_project_dir);
            if (!path_string.empty() && std::filesystem::exists(path_string) && !std::string(m_project_name).empty()) {
                auto project = std::make_shared<ICE::Project>(path_string, m_project_name);
                project->CreateDirectories();
                ImGui::CloseCurrentPopup();
                done(DialogResult::Ok);
            }
        } else if (node.arg<std::string>("id") == "btn_cancel") {
            ImGui::CloseCurrentPopup();
            done(DialogResult::Cancel);
        } else if (node.arg<std::string>("id") == "btn_browse_directory") {
            auto folder = open_native_folder_dialog();
            std::strncpy(m_project_dir, folder.c_str(), 511);
        }
    }

    std::string getProjectName() const { return std::string(m_project_name); }

    std::string getProjectDirectory() const { return std::string(m_project_dir); }

   private:
    char m_project_name[512] = {0};
    char m_project_dir[512] = {0};

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
