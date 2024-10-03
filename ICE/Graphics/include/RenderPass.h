#pragma once

namespace ICE {
class RenderPass {
   public:
    virtual ~RenderPass() = default;
    virtual void execute() = 0;
};
}  // namespace ICE