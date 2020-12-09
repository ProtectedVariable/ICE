#version 330 core
#define ICE_MAX_LIGHTS (256)

out vec4 frag_color;

struct Light {
    vec3 position;
    vec3 rotation;
    vec3 color;
};

struct Material {
    vec3 albedo;
    vec3 specular;
    vec3 ambient;
    float alpha;
};

uniform vec3 ambient_light;
uniform Light lights[ICE_MAX_LIGHTS];
uniform int light_count;
uniform Material material;


in vec3 fnormal;
in vec3 fposition;
in vec3 fview;

vec3 pointLight(Light light) {
    vec3 rcolor = vec3(0.0);

    //diffuse
    vec3 n = normalize(fnormal);
    vec3 light_direction = normalize(light.position - fposition);
    float diff = max(dot(n, light_direction), 0.0);
    rcolor += light.color * (diff * material.albedo);

    //specular
    vec3 view_direction = normalize(fview - fposition);
    vec3 reflection_direction = reflect(-light_direction, n);
    float spec = pow(max(dot(view_direction, reflection_direction), 0.0), material.alpha);
    rcolor += light.color * (spec * material.specular);

    return rcolor;
}

void main() {
    //ambient
    vec3 color_accumulator = vec3(material.ambient*ambient_light);
    for(int i = 0; i < light_count; i++) {
        color_accumulator += pointLight(lights[i]);
    }
    frag_color = vec4(color_accumulator, 1.0);
}