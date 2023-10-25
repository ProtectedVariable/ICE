#include <XMLDynamicBind.h>
#include <XMLEventHandler.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>

class Handler : public ImXML::XMLEventHandler {
    virtual void onNodeBegin(ImXML::XMLNode& node) override {}

    virtual void onNodeEnd(ImXML::XMLNode& node) override {}

    virtual void onEvent(ImXML::XMLNode& node) override {
        if (node.args["id"] == "btn0") {
            //Open file dialog
        }
    }
};

class ProjectSelectionWindow {
    ProjectSelectionWindow() {
        ImXML::XMLReader reader = ImXML::XMLReader();
        ImXML::XMLTree tree = reader.read("XML/ProjectSelection.xml");
        ImXML::XMLRenderer renderer;
        float float0;
        char buf[512] = {0};
        float color0[3] = {0};
        float color1[4] = {0};
        renderer.addDynamicBind(std::string("str0"), {.ptr = buf, .size = 512});
        Handler handler;
    }
};
