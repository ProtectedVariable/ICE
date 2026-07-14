layout(std140, binding = 0) uniform SceneData {
    mat4 uProjection;
    mat4 uView;
    vec4 uCameraPos;  // world-space camera position (xyz)
};