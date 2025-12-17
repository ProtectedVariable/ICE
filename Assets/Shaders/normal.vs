#version 420 core

#include "vert_uniforms.glsl"

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

uniform mat4 model;

out vec3 f_normal;

void main() {
    f_normal = abs(normal);
    gl_Position = uProjection * uView * model * vec4(vertex, 1.0);
}