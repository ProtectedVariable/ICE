#pragma once

// The old UI class (a window-bound immediate renderer with per-element static shaders) has been
// superseded by UIManager, which owns its resources, renders as a render-graph present-time pass,
// and hit-tests pointer input. Include the pieces you need directly.
#include "UIManager.h"
#include "UIRenderPass.h"
#include "UILabel.h"
#include "UIRect.h"
