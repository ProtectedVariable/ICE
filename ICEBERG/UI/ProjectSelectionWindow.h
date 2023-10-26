#include <XMLDynamicBind.h>
#include <XMLEventHandler.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>

#include "UIElement.h"

class Handler : public ImXML::XMLEventHandler {
   public:
    virtual void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.args["id"] == "window") {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
        }
    }

    virtual void onNodeEnd(ImXML::XMLNode& node) override {
        if (node.args["id"].starts_with("pr")) {
            if (ImGui::IsItemHovered()) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
            }
        }
    }

    virtual void onEvent(ImXML::XMLNode& node) override {
        if (node.args["id"] == "btn0") {
            //Open file dialog
        }
    }

   private:
    std::string hovered_id = "__NO_ID";
};

class ProjectSelectionWindow : public UIElement {
   public:
    ProjectSelectionWindow() { renderer.addDynamicBind("project_name", {project_name, 512, ImXML::XMLDynamicBindType::Chars}); }

    void render() override { renderer.render(tree, handler); }

   private:
    ImXML::XMLReader reader;
    ImXML::XMLTree tree = reader.read("XML/ProjectSelection.xml");
    ImXML::XMLRenderer renderer;
    char project_name[512] = {0};
    Handler handler;
};
