#include "GeometryPass.h"

namespace ICE {
GeometryPass::GeometryPass(const GraphicsFactory& factory) {
    m_framebuffer = factory.createFramebuffer({0, 0, 0});
}

}