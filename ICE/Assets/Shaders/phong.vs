#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 fnormal;
out vec3 fposition;
out vec3 fview;
out vec2 ftex_coords;

void main() {
    fview = -view[3].xyz;
    fnormal = normalize(mat3(transpose(inverse(model))) * normal);
    fposition = (model * vec4(vertex, 1.0)).xyz;
    ftex_coords = tex_coords;
    gl_Position = projection * view * model * vec4(vertex, 1.0);
}