#version 420 core

#include "vert_uniforms.glsl"
#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 bone_ids;
layout (location = 6) in vec4 bone_weights;

uniform mat4 model;
uniform mat4 bonesTransformMatrices[MAX_BONES];
uniform mat4 bonesOffsetMatrices[MAX_BONES];

out vec3 fnormal;
out vec3 ftangent;
out vec3 fbitangent;
out vec3 fposition;
out vec3 fview;
out vec2 ftex_coords;

void main() {
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    vec3 totalTangent = vec3(0.0f);
    vec3 totalBitangent = vec3(0.0f);
	
	if(bone_ids == ivec4(-1)) {
		totalPosition = vec4(vertex, 1.0f);
        totalNormal = normal;
		totalTangent = tangent; 
		totalBitangent = bitangent;
	} else {
		for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
			if(bone_ids[i] == -1) continue;
		   
			if(bone_ids[i] >= MAX_BONES) {
				totalPosition = vec4(vertex, 1.0f);
				totalNormal = normal;
				totalTangent = tangent; 
				totalBitangent = bitangent;
				break; 
			}
			
			mat4 finalBonesMatrix = bonesTransformMatrices[bone_ids[i]] * bonesOffsetMatrices[bone_ids[i]];
			
			totalPosition += finalBonesMatrix * vec4(vertex, 1.0f) * bone_weights[i];
			
			mat3 normalMatrix = mat3(transpose(inverse(finalBonesMatrix)));
			totalNormal += normalMatrix * normal * bone_weights[i];
			totalTangent += normalMatrix * tangent * bone_weights[i];
			totalBitangent += normalMatrix * bitangent * bone_weights[i];
		}
	}


    fposition = (model * totalPosition).xyz; 
	mat3 normalModelMatrix = mat3(transpose(inverse(model)));
    fnormal = normalize(normalModelMatrix * totalNormal); 
    ftangent = normalize(normalModelMatrix * totalTangent); 
    fbitangent = normalize(normalModelMatrix * totalBitangent); 

    gl_Position = uProjection * uView * model * totalPosition;

    fview = (inverse(uView) * vec4(0, 0, 0, 1)).xyz;
    ftex_coords = tex_coords;
}