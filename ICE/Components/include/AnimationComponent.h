#pragma once

#include <string>

namespace ICE {
struct AnimationComponent {
    std::string currentAnimation;
    double currentTime = 0.0;
    double speed = 1.0;
    bool playing = true;
    bool loop = true;
};
}  // namespace ICE