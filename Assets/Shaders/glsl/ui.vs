#version 420 core
layout (location = 0) in vec2 aPos;

out vec2 fUV;

uniform mat4 projection;
uniform mat4 model;

// Sub-rectangle of the bound texture to sample: the whole texture by default, a single glyph's
// cell when drawing text from the font atlas.
uniform vec2 uUVOffset;
uniform vec2 uUVScale;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    fUV = uUVOffset + aPos * uUVScale;
}
