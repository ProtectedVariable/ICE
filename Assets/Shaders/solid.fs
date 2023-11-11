#version 330 core

out vec4 frag_color;

uniform vec3 uAlbedo;

void main() {   
    frag_color = vec4(uAlbedo, 1.0);
}