#ifndef PBR_GLSL
#define PBR_GLSL

#include "structures.glsl"

const float PI = 3.1415926535f;
const float gamma = 2.2f;

float distributionGGX(vec3 N, vec3 H, float roughness)
{      
    float a2     = roughness * roughness;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
	
    float nom    = a2;
    float denom  = NdotH2 * (a2 - 1.0f) + 1.0f;
    denom        = PI * denom * denom;
	
    return nom / max(denom, 0.001f);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float k = roughness + 1.0f;
    k = (k * k) / 8.0f;
    
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1  = geometrySchlickGGX(NdotV, roughness);
    float ggx2  = geometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

vec3 shade(vec3 I, vec3 V, vec3 N, vec3 Li, ShadingMaterial material)
{
    float roughness    = material.roughness;
    float metalness    = material.metalness;
    vec3 baseColor     = material.baseColor;

    vec3 H          = normalize(I + V);
    float NdotI     = max(dot(N, I), 0.0f);
    float NdotV     = max(dot(N, V), 0.0f);
    float NdotH     = max(dot(N, H), 0.0f);
    float HdotI     = max(dot(H, I), 0.0f);
    
    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, baseColor, metalness);

    float NDF = distributionGGX(N, H, roughness);
    vec3 F    = fresnelSchlick(HdotI, F0);
    float G   = geometrySmith(N, V, I, roughness);
    
    vec3 norm    = NDF * F * G;
    float denorm = 4.0f * NdotV * NdotI;
    vec3 kd      = (1.0f - F) * (1.0f - metalness);   
   
    vec3 fs = norm / max(denorm, 0.0001f);
    vec3 fd = kd * (baseColor / PI);

    vec3 Lo = (fd + fs) * Li * NdotI;
    
    return Lo;
}

#endif