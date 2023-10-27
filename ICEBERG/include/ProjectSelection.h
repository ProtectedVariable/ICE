#pragma once

#include "Controller.h"
#include <UI/ProjectSelectionWidget.h>
class ProjectSelection : public Controller {
   public:
    void update() override;

   private:
    ProjectSelectionWidget ui;
};
