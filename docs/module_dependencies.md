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
math  storage  util  components
                 │        │
                 │      entity
                 │        │
                 └──────► asset            (CPU-only: Mesh/Material/Texture data)
                            │
                        rhi / renderer      (graphics, graphics_api; GPU types live here)
                            │
                          scene             (owns a Registry + scene graph)
                            │
                          system            (RenderSystem/AnimationSystem/... over a scene)
                            │
                           io
                            │
                          core
```

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
