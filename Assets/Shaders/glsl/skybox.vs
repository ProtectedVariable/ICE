#version 420 core

#include "vert_uniforms.glsl"


layout (location = 0) in vec3 aPos;

out vec3 TexCoords;


void main()
{
    TexCoords = aPos;
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
}