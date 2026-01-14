#pragma once

namespace ICE {
class GPUTexture {
   public:
    virtual void bind(uint32_t slot) const = 0;
};
}  // namespace ICE
