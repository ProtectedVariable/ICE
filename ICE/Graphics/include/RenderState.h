#pragma once

namespace ICE {
// Depth comparison function for a draw. Less is the default; LEqual is needed by the
// skybox (its fragments sit exactly at the far plane via the z=w trick).
enum class DepthFunc { Less, LEqual };
}  // namespace ICE
