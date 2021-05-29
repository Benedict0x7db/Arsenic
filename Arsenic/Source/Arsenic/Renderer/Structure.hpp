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

    struct Vertex
    {
        math::vec3f position;
        math::vec3f normal;
        math::vec2f uv;
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

    struct RenderObject
    {
        math::mat4f transformMatrix;
        int materialIndex = 0;
        int pad1;
        int pad2;
        int pad3;
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

    struct CameraBuffer
    {
        math::vec4f cameraPos;
        math::mat4f view;
        math::mat4f proj;
        math::mat4f invView;
        math::mat4f invProj;    
        float fov;
        float znear;
        float aspect;
        int samplerPerPixel;
    };
    
    struct SceneBuffer
    {
        int numLights = 0;
        float maxRayDepth = std::numeric_limits<float>::max();
    };

    enum class BufferType : uint8_t
    {
        None = 0,
        Vertex,
        Index,
        Uniform,
        Storage,
        Buffer
    };

    enum class BufferUsage : uint8_t
    {
        None = 0,
        Static,
        Dynamic
    };


    enum class TextureType
    {
        None = 0,
        T2D,
        Cube
    };

    enum class TextureFormat
    {
        None = 0,
        RGBAU8,
        RGBAF32,
        RGBAU8SRGB,
        RGBAF32SRGB,
        D24SU8
    };

    
    struct BufferDesc
    {
        std::size_t size;
        BufferType type;
        BufferUsage usage;
    };

    struct TextureDesc
    {
        uint32_t width;
        uint32_t height;
        uint32_t mipLevels;
        uint32_t arrayLayers;
        uint32_t sampleCount;
        TextureType type;
        TextureFormat format;
    };
    
    struct VulkanBuffer
    {
        VkBuffer vkBuffer;
        VkDeviceSize size;
        VkBufferUsageFlags bufferUsage;
        VkDeviceAddress deviceAddress;
        uint8_t *pMappedPointer;
        VmaAllocation vmaAllocation;
    };

    struct VulkanImage
    {
        VkImage vkImage;
        VkImageView vkImageView;
        uint32_t width;
        uint32_t height;
        uint32_t arrayLayers;
        uint32_t mipLevels;
        VkFormat vkFormat;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VmaAllocation vmaAllocation;
    };

    struct RawTextureInfo
    {
        uint32_t width;
        uint32_t height;
        uint32_t numChannels;
        uint8_t *pRawTextureData;
    };
}