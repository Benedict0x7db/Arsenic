#pragma once

#include "Arsenic/Renderer/VulkanHeader.hpp"

namespace arsenic
{
    struct VulkanContext;

    struct VulkanBuffer
    {
        VkBuffer vkBuffer;
        VkDeviceSize size;
        VkBufferUsageFlags bufferUsages;
        VkMemoryPropertyFlags memoryProperties;
        VkDeviceAddress deviceAddress;
        uint8_t *pMappedPointer;
        VmaAllocation vmaAllocation;
    };


    VulkanBuffer createBuffer(const VulkanContext &vulkanContext, const VkBufferUsageFlags bufferUsages, const VkMemoryPropertyFlags memoryProperties,
                const std::size_t size, const void *pData = nullptr);
        
    void destroyBuffer(const VulkanContext &vulkanContext, VulkanBuffer &vulkanBuffer);
    
    void copyBufferToBufferAndSubmit(const VulkanContext &vulkanContext, const VkBuffer src, const VkBuffer dstBuffer, const VkDeviceSize srcOffset, 
                    const VkDeviceSize dstOffset, const VkDeviceSize size);

}