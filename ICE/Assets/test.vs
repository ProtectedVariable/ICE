#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 f_normal;

void main() {
    f_normal = normal;
    gl_Position = projection * view * model * vec4(vertex, 1.0);
}