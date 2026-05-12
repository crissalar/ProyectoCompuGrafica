#version 330 core
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

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 eye;
uniform mat4 gBones[100];

out vec3 vertexPosition_cameraspace;
out vec3 Normal_cameraspace;

void main()
{
    // Aplicar skinning (transformación de huesos)
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

    // Aplicar la transformación de huesos a la posición del vértice
    vec4 PosL = BoneTransform * vec4(aPos, 1.0f);
    
    // Cálculo de la posición final
    gl_Position = projection * view * model * PosL;
    
    // Pasar coordenadas de textura al fragment shader
    TexCoords = aTexCoords;
    
    // Calcular posición del vértice en espacio de cámara para cálculos de iluminación
    vertexPosition_cameraspace = (view * model * PosL).xyz;
    
    // Transformar la normal considerando el skinning
    // Importante: Para las normales, no aplicamos la componente de traslación
    vec3 transformedNormal = mat3(BoneTransform) * aNormal;
    
    // Transformar normal al espacio de cámara
    Normal_cameraspace = (view * model * vec4(transformedNormal, 0.0)).xyz;
    
    // Normal original para cálculos adicionales
    ex_N = transformedNormal;
}