//
// Created by Thomas Ibanez on 09.12.20.
//

#ifndef ICE_PROJECTSELECTORWINDOW_H
#define ICE_PROJECTSELECTORWINDOW_H

namespace ICE {
    class ICEEngine;

    class ProjectSelectorWindow {
    public:
        void render();

        ProjectSelectorWindow(ICEEngine *engine);

    private:
        ICEEngine* engine;
    };
}

#endif //ICE_PROJECTSELECTORWINDOW_H
