# Module dependency layering (P7)

ICE is split into modules that link each other via `target_link_libraries`. That graph **must be a
DAG**: a module may only depend on lower layers. Cycles make modules impossible to build/test in
isolation and force layering inversions (e.g. a CPU-asset module reaching up into the renderer).

## Enforcement

`cmake/module_dependency_check.cmake` parses every module's `CMakeLists.txt`, builds the internal
link graph, and fails if any dependency cycle exists that is **not** in its documented baseline.
It runs as the `module_cycle_check` CTest test and standalone (no build required):

```
cmake -DICE_DIR=<repo>/ICE -P cmake/module_dependency_check.cmake
```

- **Green** when there are no cycles beyond the tracked baseline.
- **Red** (build/test failure) the moment a *new* cycle is introduced — e.g. making `math` (a leaf)
  link `graphics` is rejected because `graphics` already reaches `math`.

The baseline exists so the guard can be adopted immediately while the pre-existing debt is paid down
per-edge. **Every removed cycle should also be removed from `BASELINE_BACK_EDGES`** so the guard
tightens over time; the check prints which baseline edges are now resolved.

## Target DAG (low → high)

```
math  storage  util  components  container
                 │        │          │
                 │      entity       │  (generic containers; links NOTHING)
                 │        │          │
                 └──────► asset ◄────┘   (CPU-only: Mesh/Material/Texture/AudioClip data)
                          │   │
      rhi / renderer ◄────┘   └────► audio          (device/voice/mixer abstraction)
   (graphics, graphics_api;              │
    GPU types live here)          audio_api_openal  (OpenAL Soft backend)
                            │           │
                          scene         │
                            │           │
                          system ◄──────┘  (RenderSystem/AnimationSystem/AudioSystem/... )
                            │
                           io
                            │
                          core
```

### `container`

Dependency-free leaf holding generic containers (`HandlePool`). It exists because two layers need
the same generational-handle pool: `graphics` (GPU resource handles) and `audio` (the voice pool).
`util` would have been the natural home, but it is **not** a leaf — `EngineHelper.h` includes
`ICEEngine.h`, so `util` reaches all the way to `core` (this is the tracked `util → graphics`
back-edge below). Putting `HandlePool` there would have forced `graphics → util` and closed a new
cycle. Once the `util → graphics` debt is paid, folding `container` back into `util` is reasonable.

### `audio` / `audio_api_openal`

`audio` is the backend-agnostic layer (device, voices, mixer buses, `AudioRegistry`). It links
`assets`, `math` and `container` and deliberately **does not** link `graphics`, `scene` or
`system` — the only scene coupling in the audio stack lives in `AudioSystem`, which sits in
`system` alongside the other concrete systems.

Note there is intentionally no `audio_api` meta-target mirroring `graphics_api`: that pairing is
one of the baseline cycles below (`graphics_api ↔ graphics_api_OpenGL`). Backends link `audio` in
one direction only, and consumers link the backend they want.

## Baseline cycles and how to break them (staged, each build-validated)

| Back-edge(s) | Root cause | Fix | Notes |
|---|---|---|---|
| `assets ↔ graphics` | `GPURegistry` (and GPU upload) live in `assets` but need the graphics factory; `AssetBank.h` also has an unused `<GraphicsFactory.h>` include | Move `GPURegistry`/`GPUMesh`/`GPUTexture` to the render layer; drop the stale include so `assets` is CPU-only | **Co-moves with P8** (GPU handles). Keystone. |
| `graphics ↔ scene` | *(stale — now removed at the include level)* `ForwardRenderer` had unused `<Scene.h>`/`<Registry.h>` includes | Drop `scene` from `Graphics/CMakeLists`; add a direct `scene` link to `system` (and `util`'s helpers) which currently reach `scene` only through `graphics` | Graphics no longer *uses* scene/system (verified); only the CMake edge and downstream transitive users remain. |
| `scene ↔ system` | `Scene` owns a `Registry` (in `system`) while `system` operates on `Scene` | Move the ECS core (`Registry`, `EntityHandle`, `System`/`SystemManager`) into a low ECS layer beneath `scene`, leaving only the concrete systems in `system` | Splits the giant SCC into two once `graphics→scene` is gone. |
| `util → graphics` | `EngineHelper.h`/`EntityHelper.h` live in `util` but reference graphics/core types | Relocate those helpers out of `util` (they are superseded by `EntityHandle`/the engine facade) | `util` should be a leaf utility layer. |
| `graphics_api ↔ graphics_api_OpenGL` | Meta-target and its backend impl reference each other | Make the backend depend on the interface only (one direction), or fold the meta-target | Lowest priority; contained. |

## Rule of thumb

Before adding `target_link_libraries(<A> ... <B>)`, ask whether `B` can already reach `A`. If so,
you are closing a cycle — the type you need probably belongs in a lower layer. The `module_cycle_check`
test will reject it.
