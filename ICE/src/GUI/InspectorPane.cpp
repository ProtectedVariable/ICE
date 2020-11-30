//
// Created by Thomas Ibanez on 30.11.20.
//

#include <ImGUI/imgui.h>
#include "InspectorPane.h"

namespace ICE {
    void ICE::InspectorPane::render() {

        ImGui::Begin("Inspector");
        if(engine->getSelected() != nullptr) {
            componentRenderer.render(engine->getSelected()->getComponent<TransformComponent>());
        }
        ImGui::End();
    }

    InspectorPane::InspectorPane(ICEEngine *engine): engine(engine), componentRenderer(UIComponentRenderer()) {}
}
