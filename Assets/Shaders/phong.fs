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
    bool use_diffuse_map;
    sampler2D diffuse_map;
    bool use_ambient_map;
    sampler2D ambient_map;
    bool use_specular_map;
    sampler2D specular_map;
    bool use_normal_map;
    sampler2D normal_map;
};

uniform vec3 ambient_light;
uniform Light lights[ICE_MAX_LIGHTS];
uniform int light_count;
uniform Material material;

in vec3 fnormal;
in vec3 fposition;
in vec3 fview;
in vec2 ftex_coords;

vec3 normal;

vec3 pointLight(Light light) {
    vec3 rcolor = vec3(0.0);

    //diffuse
    vec3 n = normalize(normal);
    vec3 light_direction = normalize(light.position - fposition);
    float diff = max(dot(n, light_direction), 0.0);
    vec3 diffuse_color = material.albedo;
    if(material.use_diffuse_map) {
        diffuse_color *= texture(material.diffuse_map, ftex_coords).xyz;
    }
    rcolor += light.color * (diff * diffuse_color);

    //specular
    vec3 view_direction = normalize(fview - fposition);
    vec3 reflection_direction = reflect(-light_direction, n);
    float spec = pow(max(dot(view_direction, reflection_direction), 0.0), material.alpha);
    vec3 specular_color = material.specular;
    if(material.use_specular_map) {
        specular_color *= texture(material.specular_map, ftex_coords).xyz;
    }
    rcolor += light.color * (spec * specular_color);

    return rcolor;
}


vec3 colorToNormal(vec3 color) {
    return normalize(vec3(color.x * 2 - 1, color.y * 2 - 1, color.z * 2 - 1));
}


void main() {
    if(material.use_normal_map) {
        normal = normalize(fnormal + (fnormal * colorToNormal(texture(material.normal_map, ftex_coords).xyz)));
    } else {
        normal = fnormal;
    }
    //ambient
    vec3 color_accumulator = vec3(material.ambient*ambient_light);
    if(material.use_ambient_map) {
        color_accumulator *= texture(material.ambient_map, ftex_coords).xyz;
    }
    for(int i = 0; i < light_count; i++) {
        color_accumulator += pointLight(lights[i]);
    }
    if(light_count == 0) {
        frag_color = vec4(material.albedo, 1.0);
        if(material.use_diffuse_map) {
           frag_color *= texture(material.diffuse_map, ftex_coords);
        }
    } else {
        frag_color = vec4(color_accumulator, 1.0);
    }
}