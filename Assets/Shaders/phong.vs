#version 420 core

#include "vert_uniforms.glsl"
#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;
layout (location = 5) in ivec4 bone_ids;
layout (location = 6) in vec4 bone_weights;

uniform mat4 model;
uniform mat4 bonesTransformMatrices[MAX_BONES];
uniform mat4 bonesOffsetMatrices[MAX_BONES];

out vec3 fnormal;
out vec3 fposition;
out vec3 fview;
out vec2 ftex_coords;

void main() {
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
	
	if(bone_weights == ivec4(-1)) {
		totalPosition = vec4(vertex, 1.0f);
        totalNormal = normal;
	} else {
		for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
			if(bone_ids[i] == -1) continue;
		   
			if(bone_ids[i] >= MAX_BONES) {
				totalPosition = vec4(vertex, 1.0f);
				totalNormal = normal;
				break; 
			}
			
			mat4 finalBonesMatrix = bonesTransformMatrices[bone_ids[i]] * bonesOffsetMatrices[bone_ids[i]];
			
			vec4 localPosition = finalBonesMatrix * vec4(vertex, 1.0f);
			totalPosition += localPosition * bone_weights[i];
			
			vec3 localNormal = mat3(finalBonesMatrix) * normal;
			totalNormal += localNormal * bone_weights[i];
		}
	}


    fposition = (model * totalPosition).xyz; 
    fnormal = normalize(mat3(transpose(inverse(model))) * totalNormal); 

    gl_Position = uProjection * uView * model * totalPosition;

    fview = (inverse(uView) * vec4(0, 0, 0, 1)).xyz;
    ftex_coords = tex_coords;
}