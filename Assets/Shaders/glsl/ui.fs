#version 420 core

out vec4 fragColor;

in vec2 fUV;

uniform vec4 uColor;
uniform sampler2D uTexture;

// 0 = solid colour, 1 = colour modulated by an RGBA texture, 2 = text (texture .r is coverage).
uniform int uMode;

void main() {
    if (uMode == 2) {
        fragColor = vec4(uColor.rgb, uColor.a * texture(uTexture, fUV).r);
    } else if (uMode == 1) {
        fragColor = uColor * texture(uTexture, fUV);
    } else {
        fragColor = uColor;
    }
}
