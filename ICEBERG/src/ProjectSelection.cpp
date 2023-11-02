#include "ProjectSelection.h"

#include <dialog.h>

ProjectSelection::ProjectSelection(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {

    auto projects_list = m_engine->getConfig().getLocalProjects();
    std::vector<ProjectView> view(projects_list->size());
    std::transform(projects_list->begin(), projects_list->end(), view.begin(), [](const ICE::Project &project) {
        ProjectView v;
        v.name = project.getName();
        v.path = project.getBaseDirectory().string();
        return v;
    });
    ui.setProjects(view);

    ui.registerCallback("create_clicked", [this]() {
        auto folder = open_native_folder_dialog();
        m_done = true;
        m_engine->setProject(std::make_shared<ICE::Project>(folder, ui.getProjectName()));
        m_engine->getProject()->CreateDirectories();
    });
    ui.registerCallback("project_selected", [this](int index) {
        auto project = m_engine->getConfig().getProjectAt(index);
        m_engine->setProject(std::make_shared<ICE::Project>(*project));
        m_engine->getProject()->loadFromFile();
        m_done = true;
    });
}

bool ProjectSelection::update() {
    ui.render();
    return m_done;
}

ProjectView ProjectSelection::getSelectedProject() const {
    return m_selected_project;
}
