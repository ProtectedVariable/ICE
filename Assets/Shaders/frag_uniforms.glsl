#define MAX_LIGHTS 16

struct Light {
    vec3 position;         
    vec3 rotation;         
    vec3 color;            
    float distance_dropoff;
    int type;
};

layout(std140, binding = 1) uniform SceneLightsUBO {
    Light lights[MAX_LIGHTS];
    vec4 ambient_light;
    int light_count;
};