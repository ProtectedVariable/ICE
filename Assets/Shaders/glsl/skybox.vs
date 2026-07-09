#version 420 core

#include "vert_uniforms.glsl"


layout (location = 0) in vec3 aPos;

out vec3 TexCoords;


void main()
{
    TexCoords = aPos;
    // Strip translation from the view so the skybox stays centered on the camera, and use
    // the z=w trick so its depth is always 1.0 -- drawn behind everything with GL_LEQUAL.
    mat4 rotView = mat4(mat3(uView));
    vec4 clipPos = uProjection * rotView * vec4(aPos, 1.0);
    gl_Position = clipPos.xyww;
}