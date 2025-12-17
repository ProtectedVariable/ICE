#version 420 core

out vec4 frag_color;

in vec3 f_normal;

void main() {
    frag_color = vec4(f_normal, 1.0);
}