# Module dependency cycle check.
#
# Parses each ICE module's CMakeLists.txt, builds the internal (ICE-to-ICE) link graph, and fails
# if any dependency cycle exists that is NOT in the documented baseline below. This makes the
# current architectural debt explicit while guaranteeing that *new* cycles break the build.
#
# Run standalone:   cmake -DICE_DIR=<path-to>/ICE -P cmake/module_dependency_check.cmake
# Or via CTest:     ctest -R module_cycle_check
#
# Target DAG (what we are driving toward, low -> high):
#   math, storage, util, components  ->  entity  ->  asset (CPU only)  ->  rhi/renderer (graphics,
#   graphics_api)  ->  scene  ->  system  ->  io  ->  core
#
# Baseline cycles still to remove (each is a tracked back-edge; see docs/module_dependencies.md):
#   * assets<->graphics  : GPURegistry lives in `assets` but needs the graphics factory. Fix by
#                          moving GPURegistry/GPUMesh/GPUTexture to the render layer (co-moves with
#                          P8) and dropping AssetBank.h's unused <GraphicsFactory.h> include.
#   * graphics<->scene    : graphics no longer *uses* scene (stale includes now removed); drop the
#                          `scene` link from Graphics/CMakeLists once `system` links `scene`
#                          directly for its SceneGraphSystem.
#   * scene<->system      : Scene owns a Registry (in `system`) while `system` operates on Scene.
#                          Fix by moving the ECS core (Registry/EntityHandle/System) below scene.
#   * util->graphics      : two misplaced helper headers (EngineHelper.h, EntityHelper.h) pull in
#                          graphics/core types; relocate them out of `util`.
#   * graphics_api<->graphics_api_OpenGL : meta-target/impl mutual reference.

cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED ICE_DIR)
    get_filename_component(ICE_DIR "${CMAKE_CURRENT_LIST_DIR}/../ICE" ABSOLUTE)
endif()

# Baseline: cycle "back-edges" (A->B where B can already reach A) that exist today. Shrinking this
# list is the P7/P8 work; the check fails on any back-edge NOT listed here.
set(BASELINE_BACK_EDGES
    "assets->graphics"
    "assets->util"
    "graphics->assets"
    "graphics->scene"
    "scene->graphics"
    "scene->system"
    "system->graphics"
    "util->graphics"
    "graphics_api->graphics_api_OpenGL"
    "graphics_api_OpenGL->graphics_api"
)

# --- 1. Discover modules (CMakeLists with add_library) and their project() target names. --------
file(GLOB_RECURSE _cmakelists LIST_DIRECTORIES false "${ICE_DIR}/*/CMakeLists.txt")

set(MODULES "")
foreach(_f ${_cmakelists})
    file(READ "${_f}" _content)
    if(NOT _content MATCHES "add_library")
        continue()  # skip test suites / executables
    endif()
    string(REGEX MATCH "project\\(([A-Za-z0-9_]+)" _pm "${_content}")
    set(_name "${CMAKE_MATCH_1}")
    if(_name STREQUAL "" OR _name STREQUAL "ICE")
        continue()  # skip the aggregate ICE interface target
    endif()
    list(APPEND MODULES "${_name}")
    set(_CONTENT_${_name} "${_content}")
endforeach()
list(REMOVE_DUPLICATES MODULES)

# --- 2. Parse each module's internal link dependencies. ----------------------------------------
foreach(_name ${MODULES})
    set(_content "${_CONTENT_${_name}}")
    # Union of every target_link_libraries(...) block (some modules have platform-conditional ones).
    string(REGEX MATCHALL "target_link_libraries\\([^)]*\\)" _blocks "${_content}")
    set(_deps "")
    foreach(_block ${_blocks})
        # Normalise separators to spaces so tokens are whitespace-delimited (avoids `graphics`
        # matching inside `graphics_api`).
        string(REGEX REPLACE "[()\t\r\n]" " " _padded " ${_block} ")
        foreach(_mod ${MODULES})
            if(NOT _mod STREQUAL _name AND _padded MATCHES " ${_mod} ")
                list(APPEND _deps "${_mod}")
            endif()
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES _deps)
    set(_DEPS_${_name} "${_deps}")
endforeach()

# --- 3. Reachability helper (iterative BFS over the dependency graph). -------------------------
function(_can_reach _start _target _out)
    set(_visited "")
    set(_work "${_start}")
    while(_work)
        list(POP_FRONT _work _node)
        if(_node STREQUAL _target)
            set(${_out} TRUE PARENT_SCOPE)
            return()
        endif()
        if(NOT _node IN_LIST _visited)
            list(APPEND _visited "${_node}")
            foreach(_s ${_DEPS_${_node}})
                if(NOT _s IN_LIST _visited)
                    list(APPEND _work "${_s}")
                endif()
            endforeach()
        endif()
    endwhile()
    set(${_out} FALSE PARENT_SCOPE)
endfunction()

# --- 4. Collect back-edges: A->B where B can reach back to A (i.e. B->*A), so A->B closes a cycle.
set(BACK_EDGES "")
foreach(_name ${MODULES})
    foreach(_dep ${_DEPS_${_name}})
        _can_reach("${_dep}" "${_name}" _closes)
        if(_closes)
            list(APPEND BACK_EDGES "${_name}->${_dep}")
        endif()
    endforeach()
endforeach()

# --- 5. Compare against the baseline. ----------------------------------------------------------
set(NEW_CYCLES "")
foreach(_e ${BACK_EDGES})
    if(NOT _e IN_LIST BASELINE_BACK_EDGES)
        list(APPEND NEW_CYCLES "${_e}")
    endif()
endforeach()

set(RESOLVED "")
foreach(_e ${BASELINE_BACK_EDGES})
    if(NOT _e IN_LIST BACK_EDGES)
        list(APPEND RESOLVED "${_e}")
    endif()
endforeach()

list(LENGTH BACK_EDGES _n_back)
list(LENGTH BASELINE_BACK_EDGES _n_base)
message(STATUS "[module-cycle-check] modules: ${MODULES}")
message(STATUS "[module-cycle-check] cyclic back-edges found: ${_n_back} (baseline allows ${_n_base})")
if(RESOLVED)
    message(STATUS "[module-cycle-check] baseline cycles now RESOLVED (remove from baseline): ${RESOLVED}")
endif()

if(NEW_CYCLES)
    message(FATAL_ERROR
        "[module-cycle-check] FAILED: new dependency cycle(s) introduced (not in baseline):\n"
        "    ${NEW_CYCLES}\n"
        "A module must not link a dependency that can link back to it. Break the cycle, or -- only\n"
        "if it is intended and tracked -- add it to BASELINE_BACK_EDGES in this file with a comment.")
endif()

message(STATUS "[module-cycle-check] OK: no new cycles beyond the tracked baseline.")
