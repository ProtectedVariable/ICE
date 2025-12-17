#version 420 core

out vec4 frag_color;

in vec3 fnormal;

void main() {
    frag_color = vec4((fnormal + 1.0) / 2.0, 1.0);
}