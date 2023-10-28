#pragma once

#include <ICEEngine.h>
#include <UI/ProjectSelectionWidget.h>

#include "Controller.h"

class ProjectSelection : public Controller {
   public:
    ProjectSelection(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;
    ProjectView getSelectedProject() const;

   private:
    ProjectSelectionWidget ui;
    bool m_done = false;
    ProjectView m_selected_project;
    std::shared_ptr<ICE::ICEEngine> m_engine;
};
