#version 330 core

uniform int objectID;

out vec4 frag_color;

void main() {
    frag_color = vec4((objectID & 0xFF) / 255.0, ((objectID & 0xFF00) >> 8) / 255, ((objectID & 0xFF0000) >> 16 / 255), 1.0);
}