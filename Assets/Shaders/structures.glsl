#ifndef STRUCTURES_GLSL
#define STRUCTURES_GLSL

struct SphereMesh
{
    vec3 center;
    float radius;
    int materialIndex;
    int pad0;
    int pad1;
    int pad2;
};

struct Light
{
    int type;       // 0 = directional, 1 = point, 2 = spotlight
    float innerCutoff;
    float outerCutoff; 
    float attenuationRadius;
    vec4 position;
    vec4 spotLightDir;
    vec4 color;
};

struct Material
{
    int baseColorMap;
    int roughnessMap;
    int metallicMap;
    int emissiveMap;
    int normalMap;
    float roughness;
    float metalness;
    int samplerIndex;
    vec4 baseColor;
    vec4 emissiveColor;
};

layout(set = 0, binding = 0) uniform SceneBuffer
{
    int numLights;
    int numSphereMesh;
    int numIndirectReflect;
    int numIndirectionRefract;
    int maxRayDepth;
} _sceneBuffer;

layout(set = 0, binding = 1) uniform CameraBuffer
{
    vec4 cameraPos;
    mat4 view;
    mat4 proj;
    mat4 invView;
    mat4 invProj;    
    float aspect;
    float fov;
    float znear;
    float zfar;
    int samplerPerPixel;
    int pad0;
    int pad1;
    int pad2;
} _cameraBuffer;

layout(set = 0, binding = 2) buffer readonly SphereMeshBuffer
{
    SphereMesh sphereMeshes[];
} _sphereMeshBuffer;

layout(set = 0, binding = 3) buffer readonly LightBuffer
{
    Light lights[];
} _lightBuffer;

layout(set = 0, binding = 4) buffer readonly MaterialBuffer
{
    Material materials[];
} _materialBuffer;

layout(set = 0, binding = 5, rgba8) uniform writeonly image2D rtrenderTarget;

layout(set = 0, binding = 6) uniform sampler2D rtRenderTarget;

layout(set = 0, binding = 7) uniform samplerCube sceneEnviromentMap;


layout(push_constant) uniform PushConstant
{
    int renderObjectIndex;
} _pushConstant;

#endif