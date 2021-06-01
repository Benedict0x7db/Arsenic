#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Renderer/VulkanContext.hpp"
#include "Arsenic/Renderer/VulkanBuffer.hpp"

namespace arsenic
{
    VulkanBuffer createBuffer(const VulkanContext &vulkanContext, const VkBufferUsageFlags bufferUsages, const VkMemoryPropertyFlags memoryProperties,
                        const VkDeviceSize size, const void *pData)
    {
        assert(size);
        
        VkBufferCreateInfo bufferCI = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCI.size = size;
        bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (bufferUsages & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
            bufferCI.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (bufferUsages & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
            bufferCI.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (bufferUsages & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
            bufferCI.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if (bufferUsages & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
            bufferCI.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        vmaAllocationCI.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VulkanBuffer stagingBuffer = {};
        stagingBuffer.size = size;
        stagingBuffer.bufferUsages = bufferUsages;

        VmaAllocationInfo vmaAllocationInfo = {};
        checkVkResult(vmaCreateBuffer(vulkanContext.vmaAllocator, &bufferCI, &vmaAllocationCI, &stagingBuffer.vkBuffer, &stagingBuffer.vmaAllocation, &vmaAllocationInfo));

        stagingBuffer.pMappedPointer = static_cast<uint8_t*>(vmaAllocationInfo.pMappedData);

        if (pData != nullptr) {
            std::memcpy(stagingBuffer.pMappedPointer, pData, size);
        }

        if (memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaAllocationCI.flags = {};

            VulkanBuffer devicalLocalBuffer = {};
            devicalLocalBuffer.size = size;       

            checkVkResult(vmaCreateBuffer(vulkanContext.vmaAllocator, &bufferCI, &vmaAllocationCI, &devicalLocalBuffer.vkBuffer, &devicalLocalBuffer.vmaAllocation, nullptr));

            if (pData != nullptr) {
                copyBufferToBufferAndSubmit(vulkanContext, stagingBuffer.vkBuffer, devicalLocalBuffer.vkBuffer, 0, 0, size);
            }

            vmaDestroyBuffer(vulkanContext.vmaAllocator, stagingBuffer.vkBuffer, stagingBuffer.vmaAllocation);
        }

        return stagingBuffer;
    }

    void destroyBuffer(const VulkanContext &vulkanContext, VulkanBuffer &vulkanBuffer)
    {
        assert(vulkanBuffer.vkBuffer);
        vmaDestroyBuffer(vulkanContext.vmaAllocator, vulkanBuffer.vkBuffer, vulkanBuffer.vmaAllocation);
    }

    void copyBufferToBufferAndSubmit(const VulkanContext &vulkanContext, const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize srcOffset, 
                        const VkDeviceSize dstOffset, const VkDeviceSize size)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        checkVkResult(vkBeginCommandBuffer(vulkanContext.tempCommandBuffer, &commandBufferBeginInfo));

        VkBufferCopy bufferCopy = {};
        bufferCopy.srcOffset = srcOffset;
        bufferCopy.dstOffset= dstOffset;
        bufferCopy.size = size;

        vkCmdCopyBuffer(vulkanContext.tempCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

        checkVkResult(vkEndCommandBuffer(vulkanContext.tempCommandBuffer));

        VkSubmitInfo submtInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submtInfo.commandBufferCount = 1;
        submtInfo.pCommandBuffers = &vulkanContext.tempCommandBuffer;

        VkFence fence = VK_NULL_HANDLE;

        VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        checkVkResult(vkCreateFence(vulkanContext.device, &fenceCI, nullptr, &fence));

        checkVkResult(vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submtInfo, fence));
        checkVkResult(vkWaitForFences(vulkanContext.device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));

        vkDestroyFence(vulkanContext.device, fence, nullptr);
    }
}