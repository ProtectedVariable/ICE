#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/InspectorWidget.h"

class Inspector : public Controller {
   public:
    Inspector(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    InspectorWidget ui;
};
