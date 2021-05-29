#pragma once

#include "Arsenic/Renderer/VulkanHeader.hpp"
#include "Arsenic/Renderer/Instance.hpp"
#include "Arsenic/Renderer/PhysicalDevice.hpp"
#include "Arsenic/Renderer/Structure.hpp"
#include "vk_mem_alloc.hpp"

struct GLFWwindow;

namespace arsenic
{
    struct VulkanContext 
    {
        Instance instance;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        PhysicalDevice physicalDevice = {};
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;
        VkQueue computerQueue = VK_NULL_HANDLE;
        VmaAllocator vmaAllocator = VK_NULL_HANDLE;

        VkCommandPool tempCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer tempCommandBuffer = VK_NULL_HANDLE;
        
        VulkanBuffer createBuffer(const BufferDesc &bufferDesc, const void *pData = nullptr);
        void destroyBuffer(VulkanBuffer &vulkanBuffer);
        
        void copyBufferToBufferAndSubmit(const VkBuffer src, const VkBuffer dstBuffer, const VkDeviceSize srcOffset, 
                        const VkDeviceSize dstOffset, const VkDeviceSize size);

        VulkanImage createImage2D(const TextureFormat format, const char *filePath);
        VulkanImage createImage2D(const TextureFormat format, const VkExtent2D extent, const uint32_t arrayLayers, const uint32_t mipLevels);
        VulkanImage createImageCube(const TextureFormat format, const char *cubeMapJsonFilePath);
        void destroyImage(VulkanImage &vulkanImage);

        VkFormat findSupportedFormat(const std::initializer_list<VkFormat> &candidates, const VkImageTiling tiling, 
                    const VkFormatFeatureFlags features) const;

        VkFormat getVkFormat(const TextureFormat format) const;

        uint32_t calculateMipLevels(const uint32_t width, const uint32_t height) const 
        { return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1; }

         //  All mipLevels image layout needs to be in transfer dst optimal
        void VulkanContext::cmdGenerateMipLevels(const VkCommandBuffer commandBuffer, const VkImage vkImage, const uint32_t mipLevels, 
                            const uint32_t arrayLayers, const VkExtent3D extent3D);

        RawTextureInfo loadRawTexture(const char *filePath) const;
        void freeRawTexture(uint8_t *pRawTextureData) const;
    };  

       
    VulkanContext createVulkanContext(GLFWwindow* pGLfWwindow, const std::string& appName);
    void destroyVulkanContext(VulkanContext &renderContext);     
}