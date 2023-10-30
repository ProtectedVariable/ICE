#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/AssetsWidget.h"

class Assets : public Controller {
   public:
    Assets(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    AssetsWidget ui;
};
