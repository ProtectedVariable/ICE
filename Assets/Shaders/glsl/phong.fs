#version 420 core

#include "frag_uniforms.glsl"

out vec4 frag_color;

struct Material {
    vec4 albedo;
    vec4 specular;
    vec4 ambient;
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

uniform Material material;

in vec3 fnormal;
in vec3 ftangent;
in vec3 fbitangent;
in vec3 fposition;
in vec3 fview;
in vec2 ftex_coords;

vec3 normal;

vec4 computeLightEffect(Light light, vec3 light_direction) {
    vec4 rcolor = vec4(0.0);

    //diffuse
    vec3 n = normalize(normal);
    float diff = max(dot(n, light_direction), 0.0);
    vec4 diffuse_color = material.albedo;
    if(material.use_diffuse_map) {
        diffuse_color *= texture(material.diffuse_map, ftex_coords);
    }
	rcolor += vec4(light.color, 1.0) * (diff * diffuse_color);
	
	if(diff > 0) {
		//specular
		vec3 view_direction = normalize(fview - fposition);
		vec3 reflection_direction = reflect(-light_direction, n);
		float spec = pow(max(dot(view_direction, reflection_direction), 0.0), material.alpha);
		vec4 specular_color = material.specular;
		if(material.use_specular_map) {
			specular_color *= texture(material.specular_map, ftex_coords);
		}
		rcolor += vec4(light.color, 1.0) * (spec * specular_color);
	}
    return rcolor;
}

vec4 applyLight(Light light) {
    if(light.type == 0) {
        vec3 light_direction = normalize(light.position - fposition);
        float distance = length(light.position - fposition);
        return computeLightEffect(light, light_direction) / (1 + light.distance_dropoff * distance * distance);
    } else if(light.type == 1) {
        return computeLightEffect(light, -normalize(light.rotation));
    } else if(light.type == 2) {
        //TODO: Spot lights
    }
    // Non-void function must return on every path (spot/unknown types fell off the end -> UB).
    return vec4(0.0);
}

// Transform the tangent-space normal-map sample into world space using a proper TBN basis
// (Gram-Schmidt re-orthogonalized), matching pbr.fs. The previous math was not a TBN
// transform at all.
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(material.normal_map, ftex_coords).xyz * 2.0 - 1.0;
    vec3 N = normalize(fnormal);
    vec3 T = normalize(ftangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}


void main() {
    if(material.use_normal_map) {
        normal = getNormalFromMap();
    } else {
        normal = fnormal;
    }
    //ambient
    vec4 color_accumulator = material.ambient * ambient_light;
    if(material.use_ambient_map) {
        color_accumulator *= texture(material.ambient_map, ftex_coords);
    }
    for(int i = 0; i < light_count; i++) {
        color_accumulator += applyLight(lights[i]);
    }
    if(light_count == 0) {
        frag_color = material.albedo;
        if(material.use_diffuse_map) {
           frag_color *= texture(material.diffuse_map, ftex_coords);
        }
    } else {
        frag_color = color_accumulator;
    }
}