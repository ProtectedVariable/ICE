//
// Created by Thomas Ibanez on 25.11.20.
//

#include "CameraComponent.h"

ICE::CameraComponent::CameraComponent(ICE::Camera *camera) : camera(camera) {}

ICE::Camera *ICE::CameraComponent::getCamera() const {
    return camera;
}

bool ICE::CameraComponent::isActive() const {
    return active;
}
