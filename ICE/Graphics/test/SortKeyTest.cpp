#include <gtest/gtest.h>

#include "RenderCommand.h"

using namespace ICE;

// The sort key is now derived from stable asset UIDs, so the same draw produces the same key every
// run (previously it hashed heap addresses). computeSortKey needs no GPU objects.

TEST(SortKeyTest, DeterministicForSameInputs) {
    RenderCommand a, b;
    a.computeSortKey(/*transparent*/ false, /*depth_sq*/ 10.0f, /*shader*/ 5, /*material*/ 3);
    b.computeSortKey(false, 10.0f, 5, 3);
    EXPECT_EQ(a.sort_key, b.sort_key);
}

TEST(SortKeyTest, DiffersByMaterialAndShader) {
    RenderCommand base, other_mat, other_shader;
    base.computeSortKey(false, 10.0f, 5, 3);
    other_mat.computeSortKey(false, 10.0f, 5, 4);
    other_shader.computeSortKey(false, 10.0f, 6, 3);
    EXPECT_NE(base.sort_key, other_mat.sort_key);
    EXPECT_NE(base.sort_key, other_shader.sort_key);
}

TEST(SortKeyTest, OpaqueGroupsByShaderThenMaterial) {
    // Same shader: material orders within the group.
    RenderCommand m3, m4;
    m3.computeSortKey(false, 10.0f, 7, 3);
    m4.computeSortKey(false, 10.0f, 7, 4);
    EXPECT_LT(m3.sort_key, m4.sort_key);

    // Shader dominates material (it occupies the higher bits), so all of shader 1's draws sort
    // before any of shader 2's regardless of material.
    RenderCommand s1, s2;
    s1.computeSortKey(false, 10.0f, 1, 0x1FFFFF);
    s2.computeSortKey(false, 10.0f, 2, 0);
    EXPECT_LT(s1.sort_key, s2.sort_key);
}

TEST(SortKeyTest, TransparentSortsAfterOpaqueAndBackToFront) {
    RenderCommand opaque, transparent;
    opaque.computeSortKey(false, 10.0f, 1, 1);
    transparent.computeSortKey(true, 10.0f, 1, 1);
    EXPECT_LT(opaque.sort_key, transparent.sort_key);  // transparent bit set -> larger key -> later

    // Transparent draws go back-to-front: a farther fragment gets a smaller key (drawn first).
    RenderCommand near_t, far_t;
    near_t.computeSortKey(true, 5.0f, 1, 1);
    far_t.computeSortKey(true, 50.0f, 1, 1);
    EXPECT_LT(far_t.sort_key, near_t.sort_key);
}
