#version 420 core

#include "vert_uniforms.glsl"
#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;
layout (location = 5) in ivec4 bone_ids;
layout (location = 6) in vec4 bone_weights;

uniform mat4 model;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec3 fnormal;
out vec3 fposition;
out vec3 fview;
out vec2 ftex_coords;

void main() {
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(bone_ids[i] == -1) 
            continue;
        if(bone_ids[i] >= MAX_BONES) 
        {
            totalPosition = vec4(vertex, 1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[bone_ids[i]] * vec4(vertex,1.0f);
        totalPosition += localPosition * bone_weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[bone_ids[i]]) * normal;
    }

    fview = (inverse(uView) * vec4(0, 0, 0, 1)).xyz;
    fnormal = normalize(mat3(transpose(inverse(model))) * normal);
    fposition = (model * vec4(vertex, 1.0)).xyz;
    ftex_coords = tex_coords;
    gl_Position = uProjection * uView * model * vec4(vertex, 1.0);
}