#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/ViewportWidget.h"

class Viewport : public Controller {
   public:
    Viewport(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    ViewportWidget ui;
};
