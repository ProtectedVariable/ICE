//
// Created by Thomas Ibanez on 25.11.20.
//

#ifndef ICE_CAMERACOMPONENT_H
#define ICE_CAMERACOMPONENT_H

#include <Graphics/Camera.h>

namespace ICE {
    struct CameraComponent {
        Camera* camera;
        bool active;
    };
}


#endif //ICE_CAMERACOMPONENT_H
