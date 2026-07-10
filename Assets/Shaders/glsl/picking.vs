#version 420 core

#include "vert_uniforms.glsl"
#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

layout (location = 0) in vec3 vertex;
layout (location = 5) in ivec4 bone_ids;
layout (location = 6) in vec4 bone_weights;

// Picking is not instanced, so the model matrix is a uniform (skinning.vs takes it as a
// per-instance vertex attribute). Skinning is applied with the same logic as skinning.vs
// so an animated model is picked at its current pose, matching what is rendered on screen.
uniform mat4 model;
uniform mat4 bonesTransformMatrices[MAX_BONES];

void main() {
    vec4 totalPosition = vec4(0.0);
    if (bone_ids == ivec4(-1)) {
        totalPosition = vec4(vertex, 1.0);
    } else {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (bone_ids[i] == -1) continue;
            if (bone_ids[i] >= MAX_BONES) {
                totalPosition = vec4(vertex, 1.0);
                break;
            }
            totalPosition += bonesTransformMatrices[bone_ids[i]] * vec4(vertex, 1.0) * bone_weights[i];
        }
    }
    gl_Position = uProjection * uView * model * totalPosition;
}
