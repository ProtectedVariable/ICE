#pragma once

#include "HandlePool.h"

namespace ICE {

// Tag-typed handles to GPU resources owned by the GPURegistry. These are the lightweight values the
// per-frame render path carries instead of shared_ptr<GPU*>: resolve them to a raw pointer once, at
// bind time, via GPURegistry::resolve(). The tags below are never instantiated -- they only make
// the three handle kinds distinct types.
class GPUMesh;
class GPUTexture;
class ShaderProgram;

using MeshHandle = Handle<GPUMesh>;
using TextureHandle = Handle<GPUTexture>;
using ShaderHandle = Handle<ShaderProgram>;

}  // namespace ICE
