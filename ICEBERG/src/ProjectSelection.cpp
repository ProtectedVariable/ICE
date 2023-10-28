#include "ProjectSelection.h"

#include <dialog.h>

ProjectSelection::ProjectSelection(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {
    ui.registerCallback("create_clicked", [this]() {
        auto folder = open_native_folder_dialog();
        m_done = true;
        m_engine->setProject(std::make_shared<ICE::Project>(folder, ui.getProjectName()));
        m_engine->getProject()->CreateDirectories();
        
    });
    ui.registerCallback("project_selected", [this]() { m_done = true; });
}

bool ProjectSelection::update() {
    ui.render();
    return m_done;
}

ProjectView ProjectSelection::getSelectedProject() const {
    return m_selected_project;
}
