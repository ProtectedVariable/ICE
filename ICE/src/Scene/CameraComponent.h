//
// Created by Thomas Ibanez on 25.11.20.
//

#ifndef ICE_CAMERACOMPONENT_H
#define ICE_CAMERACOMPONENT_H

#include <Graphics/Camera.h>

namespace ICE {
    class CameraComponent {
    public:
        CameraComponent(Camera *camera);

        Camera *getCamera() const;

        bool isActive() const;

    private:
        Camera* camera;
        bool active;
    };
}


#endif //ICE_CAMERACOMPONENT_H
