#version 420 core

#include "frag_uniforms.glsl"

struct Material {
    vec3 baseColor;
    float metallic;
    float roughness;
    float ao;

    bool hasBaseColorMap;
    sampler2D baseColorMap;

    bool hasMetallicMap;
    sampler2D metallicMap;

    bool hasRoughnessMap;
    sampler2D roughnessMap;

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
const float PI = 3.14159265359;

vec3 getNormalFromMap() {
    if(material.hasNormalMap) {
        vec3 tangentNormal = texture(material.normalMap, ftex_coords).xyz * 2.0 - 1.0;

        vec3 N   = normalize(fnormal);
        vec3 T  = normalize(ftangent);
        vec3 B  = normalize(fbitangent);
        mat3 TBN = mat3(T, B, N);

        return normalize(TBN * tangentNormal);
    } else {
        return fnormal;
    }
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main() {		
    vec3 albedo     = pow(material.hasBaseColorMap ? texture(material.baseColorMap, ftex_coords).rgb : material.baseColor, vec3(2.2));
    float metallic  = material.hasMetallicMap ? texture(material.metallicMap, ftex_coords).r : material.metallic;
    float roughness = material.hasRoughnessMap ? texture(material.roughnessMap, ftex_coords).r : material.roughness;
    float ao        = material.hasAoMap ? texture(material.aoMap, ftex_coords).r : material.ao;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(fview - fposition);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) {
        // calculate per-light radiance
        vec3 L = normalize(lights[i].position - fposition);
        vec3 H = normalize(V + L);
        float distance = length(lights[i].position - fposition);
        float attenuation = 1.0 / (1 + lights[i].distance_dropoff * distance * distance);
        vec3 radiance = lights[i].color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    //TODO IBL
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    frag_color = vec4(color, 1.0);
}