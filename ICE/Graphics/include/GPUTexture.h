#pragma once

namespace ICE {
class GPUTexture {
   public:
    virtual void bind(uint32_t slot) const = 0;
    virtual int id() const = 0;
    void* ptr() const { return reinterpret_cast<void*>(static_cast<uintptr_t>(id())); }
    virtual ~GPUTexture() = default;
};
}  // namespace ICE
