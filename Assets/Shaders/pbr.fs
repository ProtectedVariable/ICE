#version 420 core

#include "frag_uniforms.glsl"

struct Material {
    vec3 baseColor;
    float metallic;
    float roughness;
    float ao;

    bool hasBaseColorMap;
    sampler2D baseColorMap;

    bool hasMetallicRoughnessMap;
    sampler2D metallicRoughnessMap;

    bool hasNormalMap;
    sampler2D normalMap;

    bool hasAoMap;
    sampler2D aoMap;
};

uniform Material material;

in vec3 fnormal;
in vec3 ftangent;
in vec3 fbitangent;
in vec3 fposition;
in vec3 fview;
in vec2 ftex_coords;

out vec4 frag_color;

void main() {
    vec3 N = fnormal;
    vec3 V = normalize(fview - fposition);

    // Normal map
    if (material.hasNormalMap) {
        vec3 normalTex = texture(material.normalMap, ftex_coords).xyz * 2.0 - 1.0;
        mat3 TBN = mat3(ftangent, fbitangent, fnormal);
        N = normalize(TBN * normalTex);
    }

    vec3 albedo = material.baseColor;
    if (material.hasBaseColorMap)
        albedo *= texture(material.baseColorMap, ftex_coords).rgb;

    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = material.ao;
    if (material.hasAoMap)
        ao *= texture(material.aoMap, ftex_coords).r;

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < light_count; i++) {
        vec3 L = normalize(lights[i].position - fposition);
        float distance = length(lights[i].position - fposition);
        vec3 H = normalize(L + V);

        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.001);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        // Cook-Torrance specular
        float alpha = roughness * roughness;
        float D = alpha / (3.141592 * pow(NdotH*NdotH*(alpha-1)+1,2));
        float k = (alpha + 1)*(alpha +1)/8;
        float G = NdotL/(NdotL*(1-k)+k) * NdotV/(NdotV*(1-k)+k);
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = F0 + (1.0 - F0)*pow(1.0 - VdotH, 5.0);

        vec3 spec = (D * G * F)/(4*NdotV*NdotL + 0.001);
        vec3 diffuse = albedo / 3.141592 * (1.0 - metallic);

        vec3 radiance = lights[i].color / (distance*distance); 
        Lo += NdotL * radiance * (diffuse + spec);
    }

    frag_color = vec4(Lo * ao, 1.0);
}
