#pragma once

#include "Arsenic/Renderer/VulkanHeader.hpp"

namespace arsenic
{
    struct VulkanContext;

    struct VulkanImage
    {
        VkImage vkImage;
        VkImageView vkImageView;
        VkExtent3D extent;
        uint32_t arrayLayers;
        uint32_t mipLevels;
        VkFormat format;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VmaAllocation vmaAllocation;
    };

    struct VulkanImageDesc
    {
        VkImageUsageFlags imageUsageFlags;
        VkExtent3D extent;
        VkFormat format;
        uint32_t numArrayLayers;
        bool setMipLevel;

        static VulkanImageDesc create(VkImageUsageFlags imageUsageFlags, VkExtent3D extent, VkFormat format, uint32_t numArrayLayers,
                    bool setMipLevel) noexcept
        {
            VulkanImageDesc imageDesc = {};
            imageDesc.imageUsageFlags = imageUsageFlags;
            imageDesc.extent = extent;
            imageDesc.format = format;
            imageDesc.numArrayLayers = numArrayLayers;
            imageDesc.setMipLevel = setMipLevel;

            return imageDesc;
        }
    };

    VulkanImage createImage2D(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc);
    VulkanImage createCubeImage2D(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc);

    VulkanImage loadImage2DFromFile(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc, const char *imageFilePath);
    VulkanImage loadHDRImage2DFromFile(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc, const char *hdrImageFilePath);

    VulkanImage loadCubeImage2DFromFile(const VulkanContext &vulkanContext, const char *cubeJsonFilePath);
    
    VkImageView createImageView(const VulkanContext &vulkanContext, const VkImage image, const VkImageViewType viewType, 
                            const VkFormat format, const VkImageAspectFlags imageAspect, const uint32_t baseArrayLayer, 
                            const uint32_t layerCount, const uint32_t baseMipLevel, const uint32_t levelCount);
  
    void destroyImage(const VulkanContext &vulkanContext, VulkanImage &vulkanImage);
    
    // All mipLevels from baseArrayLayer to numArrayLayers - 1 image layout needs to be in transfer dst optimal
    // After returning, all mipLevels from baseArrayLayer to numArrayLayers - 1 image layout is in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    // This allows the calle to know exactly what layout is set of arraylayers is in
    void cmdGenerateMipLevels(const VkCommandBuffer commandBuffer, const VkImage vkImage, const uint32_t baseArrayLayer, const uint32_t numArrayLayers,
                    const uint32_t mipLevels, const VkExtent3D extent);

    constexpr uint32_t calculateMipLevels(const uint32_t width, const uint32_t height)  
    { return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1; }
}