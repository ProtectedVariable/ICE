#pragma once

#include <vector>

static std::vector<float> full_quad_v = {
    -1.0f, -1.0f, 0.0f,  // TOP LEFT
    1.0,   -1.0f, 0.0f,  // TOP RIGHT
    -1.0f, 1.0,   0.0f,  // BOTTOM LEFT 
    1.0,   1.0,   0.0f,  // BOTTOM RIGHT
};

static std::vector<int> full_quad_idx = {
    0, 1, 2, 2, 1, 3
};


static std::vector<float> full_quad_tx = {
    0, 0,  // TOP LEFT
    1, 0,  // TOP RIGHT
    0, 1,  // BOTTOM LEFT
    1, 1,  // BOTTOM RIGHT
    1, 0,  // TOP RIGHT
    0, 1   // BOTTOM LEFT
};
