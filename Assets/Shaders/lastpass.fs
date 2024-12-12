#version 330 core

out vec4 fragColor;

in vec2 fUV;
uniform sampler2D uTexture;

void main() {
    fragColor = texture(uTexture, fUV);
} 