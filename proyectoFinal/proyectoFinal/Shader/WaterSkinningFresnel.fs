#version 330 core

#extension GL_NV_shadow_samplers_cube : enable

// Combined fragment shader for Fresnel effects and vertex skinning
// Based on: The CG Tutorial, Nvidia developer zone
// Adaptation by: Based on PhD Sergio Teodoro-Vite's work

in vec2 TexCoords;
in vec3 ex_N; 
in vec3 EyeDirection_cameraspace;

// Incoming Fresnel reflection and refraction parameters
in vec3  vReflect;
in vec3  vRefract[3];
in float reflectionCoefficient;

out vec4 FragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Textures
uniform samplerCube cubetex;          // Environment cubemap for reflections/refractions
uniform sampler2D texture_diffuse1;   // Diffuse texture

// Lighting control
uniform bool enableFresnel = true;    // Toggle Fresnel effect
uniform bool enableLighting = true;   // Toggle Phong lighting
uniform float fresnelMix = 0.7;       // Control blend between Fresnel and regular texture/lighting

void main()
{
    // ========= BASE TEXTURE =========
    vec4 texel = texture(texture_diffuse1, TexCoords);
    
    // ========= FRESNEL CALCULATION =========
    vec4 fresnelColor;
    
    if (enableFresnel) {
        // Calculate reflection color
        vec4 reflectedColor = textureCube(cubetex, vec3(vReflect.x, vReflect.yz));
        
        // Calculate refraction color
        vec4 refractedColor;
        refractedColor.r = textureCube(cubetex, vec3(vRefract[0].x, vRefract[0].yz)).r;
        refractedColor.g = textureCube(cubetex, vec3(vRefract[1].x, vRefract[1].yz)).g;
        refractedColor.b = textureCube(cubetex, vec3(vRefract[2].x, vRefract[2].yz)).b;
        refractedColor.a = 1.0;
        
        // Combine reflection and refraction based on Fresnel coefficient
        fresnelColor = reflectionCoefficient * reflectedColor + (1.0 - reflectionCoefficient) * refractedColor;
    } else {
        fresnelColor = texel;
    }
    
    // ========= PHONG LIGHTING =========
    vec4 litColor = texel;
    
    if (enableLighting) {
        // Ambient component
        vec4 MaterialAmbientColor = vec4(0.5, 0.5, 0.5, 1.0);
        
        // Light direction (directional light from above)
        vec3 lightDirection = vec3(0.0, -1.0, 0.0);
        vec3 LightPosition_cameraspace = (view * vec4(lightDirection, 1.0)).xyz;
        vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
        vec3 l = normalize(LightDirection_cameraspace);
        
        // Diffuse component
        float intensity = clamp(dot(ex_N, l), 0, 1);
        vec4 MaterialDiffuseColor = intensity * vec4(1.0, 1.0, 1.0, 1.0);
        
        // Specular component
        vec4 LightPower = vec4(1.0, 1.0, 1.0, 1.0);
        vec3 Normal_cameraspace = (view * model * vec4(ex_N, 0)).xyz;
        vec3 n = normalize(Normal_cameraspace);
        vec3 R = reflect(-l, n);
        vec3 E = normalize(EyeDirection_cameraspace);
        float cosAlpha = clamp(dot(E, R), 0, 1);
        vec4 MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0) * LightPower * pow(cosAlpha, 5);
        
        // Combine lighting components with texture
        litColor = texel * (MaterialAmbientColor + MaterialDiffuseColor + MaterialSpecularColor);
    }
    
    // ========= FINAL COLOR CALCULATION =========
    // Mix between Fresnel effect and regular lit texture based on fresnelMix parameter
    FragColor = mix(litColor, fresnelColor, fresnelMix);
    FragColor.a = 1.0;
}