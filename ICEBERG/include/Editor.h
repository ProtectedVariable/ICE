#pragma once

#include <ICEEngine.h>
#include <UI/EditorWidget.h>

#include <memory>
#include <vector>

#include "Controller.h"

class Editor : public Controller {
   public:
    Editor(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    EditorWidget ui;
    std::vector<std::unique_ptr<Controller>> m_subpannels;
};
