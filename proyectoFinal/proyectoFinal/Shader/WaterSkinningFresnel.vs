#version 330 core

#extension GL_NV_shadow_samplers_cube : enable

// Combined shader for Fresnel effects and vertex skinning
// Based on: The CG Tutorial, Nvidia developer zone
// Adaptation by: Based on PhD Sergio Teodoro-Vite's work

layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  tangent;
layout (location = 4) in vec3  bitangent;
layout (location = 5) in vec4  bIDs1;     // N max bones per vertex
layout (location = 6) in vec4  bIDs2;     // N max bones per vertex
layout (location = 7) in vec4  bIDs3;     // N max bones per vertex
layout (location = 8) in vec4  bWeights1; // N max bones per vertex
layout (location = 9) in vec4  bWeights2; // N max bones per vertex
layout (location = 10) in vec4 bWeights3; // N max bones per vertex

out vec2 TexCoords;
out vec3 ex_N;
out vec3 EyeDirection_cameraspace;

// Fresnel outputs
out vec3  vReflect;
out vec3  vRefract[3];
out float reflectionCoefficient;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraPosition;

// Bones array for skinning
uniform mat4 gBones[100];

// Fresnel parameters
uniform float mRefractionRatio;
uniform float _Bias;
uniform float _Scale;
uniform float _Power;

void main()
{
    // ========= SKINNING PART =========
    mat4 BoneTransform = gBones[int(bIDs1[0])] * bWeights1[0];
    BoneTransform += gBones[int(bIDs1[1])] * bWeights1[1];
    BoneTransform += gBones[int(bIDs1[2])] * bWeights1[2];  
    BoneTransform += gBones[int(bIDs1[3])] * bWeights1[3];

    BoneTransform += gBones[int(bIDs2[0])] * bWeights2[0];
    BoneTransform += gBones[int(bIDs2[1])] * bWeights2[1];
    BoneTransform += gBones[int(bIDs2[2])] * bWeights2[2]; 
    BoneTransform += gBones[int(bIDs2[3])] * bWeights2[3];

    BoneTransform += gBones[int(bIDs3[0])] * bWeights3[0];
    BoneTransform += gBones[int(bIDs3[1])] * bWeights3[1];
    BoneTransform += gBones[int(bIDs3[2])] * bWeights3[2]; 
    BoneTransform += gBones[int(bIDs3[3])] * bWeights3[3];

    // Apply bone transformation to the vertex position
    vec4 PosL = BoneTransform * vec4(aPos, 1.0f);
    
    // Transform normal by bone transformation (note: normalization happens later)
    vec3 normalL = mat3(BoneTransform) * aNormal;

    // ========= POSITION CALCULATIONS =========
    // Final position calculation with model-view-projection
    gl_Position = projection * view * model * PosL;

    // Position of the vertex in worldspace after skinning
    vec3 posWorld = (model * PosL).xyz;
    
    // Normal in world space after skinning and model transformation
    vec3 normWorld = normalize(mat3(model) * normalL);
    
    // Pass the normal to the fragment shader
    ex_N = normWorld;

    // Calculate eye direction for lighting
    vec3 vertexPosition_cameraspace = (view * model * PosL).xyz;
    EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

    // ========= FRESNEL PART =========
    // Incident vector from eye to surface
    vec3 I = normalize(posWorld - cameraPosition);

    // Calculate reflection and refraction vectors
    vReflect = reflect(I, normWorld);
    
    // Calculate refraction for RGB channels with slightly different refraction indices
    vRefract[0] = refract(I, normWorld, mRefractionRatio * 1.0f);  // Red
    vRefract[1] = refract(I, normWorld, mRefractionRatio * 0.99f); // Green
    vRefract[2] = refract(I, normWorld, mRefractionRatio * 0.98f); // Blue

    // Schlick Approximation for Fresnel equation
    reflectionCoefficient = max(0, min(1, _Bias + _Scale * pow(1.0f + dot(I, normWorld), _Power)));
    
    // Pass texture coordinates to the fragment shader
    TexCoords = aTexCoords;
}