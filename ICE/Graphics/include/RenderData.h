#pragma once

#include <vector>

// Fullscreen-quad geometry in NDC. `inline` gives a single shared definition across
// translation units instead of one static copy per TU.
inline const std::vector<float> full_quad_v = {
    -1.0f, -1.0f, 0.0f,  // bottom-left
    1.0f,  -1.0f, 0.0f,  // bottom-right
    -1.0f, 1.0f,  0.0f,  // top-left
    1.0f,  1.0f,  0.0f,  // top-right
};

inline const std::vector<int> full_quad_idx = {
    0, 1, 2, 2, 1, 3
};

// One UV per vertex (the quad has 4 vertices); the previous array had 6 pairs, so the
// last two were dead.
inline const std::vector<float> full_quad_tx = {
    0, 0,  // bottom-left
    1, 0,  // bottom-right
    0, 1,  // top-left
    1, 1,  // top-right
};
