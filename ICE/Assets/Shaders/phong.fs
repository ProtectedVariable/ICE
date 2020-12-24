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
    bool useDiffuseMap;
    sampler2D diffuseMap;
    bool useAmbientMap;
    sampler2D ambientMap;
    bool useSpecularMap;
    sampler2D specularMap;
    bool useNormalMap;
    sampler2D normalMap;
};

uniform vec3 ambient_light;
uniform Light lights[ICE_MAX_LIGHTS];
uniform int light_count;
uniform Material material;

in vec3 fnormal;
in vec3 fposition;
in vec3 fview;
in vec2 ftex_coords;

vec3 pointLight(Light light) {
    vec3 rcolor = vec3(0.0);

    //diffuse
    vec3 n = normalize(fnormal);
    vec3 light_direction = normalize(light.position - fposition);
    float diff = max(dot(n, light_direction), 0.0);
    vec3 diffuse_color = material.albedo;
    if(material.useDiffuseMap) {
        diffuse_color *= texture(material.diffuseMap, ftex_coords).xyz;
    }
    rcolor += light.color * (diff * diffuse_color);

    //specular
    vec3 view_direction = normalize(fview - fposition);
    vec3 reflection_direction = reflect(-light_direction, n);
    float spec = pow(max(dot(view_direction, reflection_direction), 0.0), material.alpha);
    vec3 specular_color = material.specular;
    if(material.useSpecularMap) {
        specular_color *= texture(material.specularMap, ftex_coords).xyz;
    }
    rcolor += light.color * (spec * specular_color);

    return rcolor;
}

void main() {
    //ambient
    vec3 color_accumulator = vec3(material.ambient*ambient_light);
    if(material.useAmbientMap) {
        color_accumulator *= texture(material.ambientMap, ftex_coords).xyz;
    }
    for(int i = 0; i < light_count; i++) {
        color_accumulator += pointLight(lights[i]);
    }
    if(light_count == 0) {
        frag_color = vec4(material.albedo, 1.0);
        if(material.useDiffuseMap) {
           frag_color *= texture(material.diffuseMap, ftex_coords);
        }
    } else {
        frag_color = vec4(color_accumulator, 1.0);
    }
}