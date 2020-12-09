#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 fnormal;
out vec3 fposition;
out vec3 fview;

void main() {
    fview = -view[3].xyz;
    fnormal = (vec4(normal, 0.0) * model).xyz;
    fposition = (model * vec4(vertex, 1.0)).xyz;
    gl_Position = projection * view * model * vec4(vertex, 1.0);
}