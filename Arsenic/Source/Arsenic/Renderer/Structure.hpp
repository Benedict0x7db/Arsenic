#pragma once

#include "Arsenic/Math/Math.hpp"
#include "Arsenic/Renderer/Handle.hpp"
#include "Arsenic/Renderer/VulkanHeader.hpp"

#include "vk_mem_alloc.hpp"

namespace arsenic
{   
    constexpr std::size_t maxFrameInFlight = 3;

    struct Frame
    {
        VkCommandPool commandPool;
        VkSemaphore imageReadySemaphore;
        VkSemaphore renderFinishSemaphore;
        VkFence frameInFlightFence;
    };
    
    template<typename T>
    struct PerFrame
    {
        std::array<T, maxFrameInFlight> value;
    };
    
    struct SceneBuffer
    {
        int numLights = 0;
        int numSphereMeshes = 0;
        int numIndirectReflect = 0;
        int numIndirectionRefract = 0;
        const float maxRayDepth = std::numeric_limits<float>::max();
    };

    struct CameraBuffer
    {
        math::vec4f cameraPos;
        math::mat4f view;
        math::mat4f proj;
        math::mat4f invView;
        math::mat4f invProj;    
        float aspect;
        float fov;
        float znear;
        float zfar;
        int samplerPerPixel;
        int pad0;
        int pad1;
        int pad2;
    };

    struct Light
    {
        int type;                           // 0 = directional, 1 = point, 2 = spotlight
        float innerCutoff;
        float outerCutoff; 
        float attenuationRadius;
        math::vec4f position;
        math::vec4f spotLightDir;
        math::vec4f color;
    };

    struct Material
    {
        int baseColorMapIndex;
        int roughnessMapIndex;
        int metalicMapIndex;
        int emissiveMapIndex;
        int normalMapIndex;
        float roughness;
        float metalness;
        int samplerIndex;
        math::vec4f baseColor;
        math::vec4f emissiveColor;
    };
    
    struct Vertex
    {
        math::vec3f position;
        math::vec3f normal;
        math::vec2f uv;
    };


    struct Mesh
    {
        std::vector<Vertex> verticles;
        std::vector<uint32_t> indicles;
        BufferHandle vertexBufferHandle = invalidHandle;
        BufferHandle indexBufferHandle = invalidHandle;
    };
    
    struct MeshObject
    {
        const Mesh *pMesh = nullptr;
        int renderObjectIndex = -1;
    };

    struct SphereMesh
    {
        math::vec3f center;
        float radius = 1.0f;
        int materialIndex = -1;
        int pad0;
        int pad1;
        int pad2;
    };
}