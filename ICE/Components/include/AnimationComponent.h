#pragma once

#include <string>

namespace ICE {
struct AnimationComponent {
    std::string currentAnimation;
    double currentTime = 0.0;
    double speed = 1.0;
    bool playing = true;
    bool loop = true;

    // Blending / crossfade
    std::string previousAnimation;
    double previousTime = 0.0;
    double blendFactor = 1.0;   // 0.0 = fully previousAnimation, 1.0 = fully currentAnimation
    double blendDuration = 0.0; // ms; 0 = instant switch
    bool blending = false;

    void playAnimation(const std::string& name, double blendDur = 0.0) {
        if (name == currentAnimation) return;
        if (blendDur > 0.0 && !currentAnimation.empty()) {
            previousAnimation = currentAnimation;
            previousTime = currentTime;
            blendFactor = 0.0;
            blendDuration = blendDur;
            blending = true;
        } else {
            blending = false;
            blendFactor = 1.0;
        }
        currentAnimation = name;
        currentTime = 0.0;
        playing = true;
    }
};
}  // namespace ICE