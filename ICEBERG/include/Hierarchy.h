#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/HierarchyWidget.h"

class Hierarchy : public Controller {
   public:
    Hierarchy(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    HierarchyWidget ui;
};
