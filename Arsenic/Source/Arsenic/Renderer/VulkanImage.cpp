#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Renderer/VulkanContext.hpp"
#include "Arsenic/Renderer/VulkanBuffer.hpp"
#include "Arsenic/Renderer/VulkanImage.hpp"

#include "nlohmann/json.hpp"
#include "stb_image.hpp"

namespace arsenic
{
    VulkanImage createImage2D(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc)
    {        
        const uint32_t arrayLayers = imageDesc.numArrayLayers;
        const uint32_t mipLevels = calculateMipLevels(imageDesc.extent.width, imageDesc.extent.height);

        VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = imageDesc.format;
        imageCI.extent = {imageDesc.extent.width, imageDesc.extent.height, 1};
        imageCI.mipLevels = imageDesc.setMipLevel ? mipLevels : 1;
        imageCI.arrayLayers = arrayLayers;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = imageDesc.imageUsageFlags;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VulkanImage vulkanImage = {};
        vulkanImage.arrayLayers =  arrayLayers;
        vulkanImage.mipLevels = mipLevels;
        vulkanImage.extent = imageDesc.extent;
        vulkanImage.format = imageDesc.format;
        vulkanImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        checkVkResult(vmaCreateImage(vulkanContext.vmaAllocator, &imageCI, &vmaAllocationCI, &vulkanImage.vkImage, &vulkanImage.vmaAllocation, nullptr));

        return vulkanImage;
    }

    VulkanImage createCubeImage2D(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc)
    {
        const uint32_t arrayLayers = imageDesc.numArrayLayers;
        const uint32_t mipLevels = imageDesc.setMipLevel ? calculateMipLevels(imageDesc.extent.width, imageDesc.extent.height) : 1;

        VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = imageDesc.format;
        imageCI.extent = {imageDesc.extent.width, imageDesc.extent.height, 1};
        imageCI.mipLevels = imageDesc.setMipLevel ? mipLevels : 1;
        imageCI.arrayLayers = arrayLayers;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = imageDesc.imageUsageFlags;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VulkanImage vulkanImage = {};
        vulkanImage.arrayLayers =  arrayLayers;
        vulkanImage.mipLevels = mipLevels;
        vulkanImage.extent = imageDesc.extent;
        vulkanImage.format = imageDesc.format;

        checkVkResult(vmaCreateImage(vulkanContext.vmaAllocator, &imageCI, &vmaAllocationCI, &vulkanImage.vkImage, &vulkanImage.vmaAllocation, nullptr));
        
        return vulkanImage;
    }

    VulkanImage loadImage2DFromFile(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc, const char *imageFilePath)
    {
        int width = 0;
        int height = 0;
        int numChannel = 0;

        auto *pRawImageData = stbi_load(imageFilePath, &width, &height, &numChannel, STBI_rgb_alpha);
        
        const uint32_t arrayLayers = imageDesc.numArrayLayers;
        const uint32_t mipLevels = imageDesc.setMipLevel ? calculateMipLevels(width, height) : 1;
        const uint32_t arrayLayerBufferSize = 4u * width * height;

        VulkanBuffer stagingBuffer = createBuffer(vulkanContext, 0, 0, arrayLayerBufferSize, pRawImageData);

        VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = imageDesc.format;
        imageCI.extent.width = static_cast<uint32_t>(width);
        imageCI.extent.height = static_cast<uint32_t>(height);
        imageCI.extent.depth = 1;
        imageCI.mipLevels = mipLevels;
        imageCI.arrayLayers = arrayLayers;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = imageDesc.imageUsageFlags;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VulkanImage vulkanImage = {};
        vulkanImage.arrayLayers = arrayLayers;
        vulkanImage.mipLevels = mipLevels;
        vulkanImage.extent = imageDesc.extent;
        vulkanImage.format = imageDesc.format;

        checkVkResult(vmaCreateImage(vulkanContext.vmaAllocator, &imageCI, &vmaAllocationCI, &vulkanImage.vkImage, &vulkanImage.vmaAllocation, nullptr));

        const VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        checkVkResult(vkBeginCommandBuffer(vulkanContext.tempCommandBuffer, &commandBufferBeginInfo));
        
        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = mipLevels;

            vkCmdPipelineBarrier(vulkanContext.tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }
        {
            VkBufferImageCopy bufferImageCopy = {};
            bufferImageCopy.imageExtent.width = static_cast<uint32_t>(width);
            bufferImageCopy.imageExtent.height = static_cast<uint32_t>(height);
            bufferImageCopy.imageExtent.depth = 1;
            bufferImageCopy.imageSubresource.baseArrayLayer = 0;
            bufferImageCopy.imageSubresource.layerCount = arrayLayers;
            bufferImageCopy.imageSubresource.mipLevel = 0;

            vkCmdCopyBufferToImage(vulkanContext.tempCommandBuffer, stagingBuffer.vkBuffer, vulkanImage.vkImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
        }
      
        cmdGenerateMipLevels(vulkanContext.tempCommandBuffer, vulkanImage.vkImage, 0, arrayLayers, mipLevels, imageCI.extent);

        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = mipLevels;

            vkCmdPipelineBarrier(vulkanContext.tempCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            vulkanImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        checkVkResult(vkEndCommandBuffer(vulkanContext.tempCommandBuffer));

        {
            VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            VkFence fence = VK_NULL_HANDLE;
            checkVkResult(vkCreateFence(vulkanContext.device, &fenceCI, nullptr, &fence));

            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &vulkanContext.tempCommandBuffer;
            checkVkResult(vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submitInfo, fence));

            checkVkResult(vkWaitForFences(vulkanContext.device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
            vkDestroyFence(vulkanContext.device, fence, nullptr);
        }

        destroyBuffer(vulkanContext, stagingBuffer);
        stbi_image_free(pRawImageData);

        return vulkanImage;
    }

    VulkanImage loadHDRImage2DFromFile(const VulkanContext &vulkanContext, const VulkanImageDesc &imageDesc, const char *hdrImageFilePath)
    {
        int width = 0;
        int height = 0;
        int numChannel = 0;

        auto *pRawImageData = stbi_loadf(hdrImageFilePath, &width, &height, &numChannel, STBI_rgb_alpha);
        
        const uint32_t arrayLayers = imageDesc.numArrayLayers;
        const uint32_t mipLevels = imageDesc.setMipLevel ? calculateMipLevels(width, height) : 1;
        const uint32_t arrayLayerBufferSize = 4u * width * height;

        VulkanBuffer stagingBuffer = createBuffer(vulkanContext, 0, 0, arrayLayerBufferSize, pRawImageData);

        VkImageCreateInfo imageCI = {};
        imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = imageDesc.format;
        imageCI.extent.width = static_cast<uint32_t>(width);
        imageCI.extent.height = static_cast<uint32_t>(height);
        imageCI.extent.depth = 1;
        imageCI.mipLevels = mipLevels;
        imageCI.arrayLayers = arrayLayers;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = imageDesc.imageUsageFlags;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VulkanImage vulkanImage = {};
        vulkanImage.arrayLayers = arrayLayers;
        vulkanImage.mipLevels = mipLevels;
        vulkanImage.extent = imageDesc.extent;
        vulkanImage.format = imageDesc.format;

        checkVkResult(vmaCreateImage(vulkanContext.vmaAllocator, &imageCI, &vmaAllocationCI, &vulkanImage.vkImage, &vulkanImage.vmaAllocation, nullptr));

        const VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        checkVkResult(vkBeginCommandBuffer(vulkanContext.tempCommandBuffer, &commandBufferBeginInfo));
        
        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = mipLevels;

            vkCmdPipelineBarrier(vulkanContext.tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }
        {
            VkBufferImageCopy bufferImageCopy = {};
            bufferImageCopy.imageExtent.width = static_cast<uint32_t>(width);
            bufferImageCopy.imageExtent.height = static_cast<uint32_t>(height);
            bufferImageCopy.imageExtent.depth = 1;
            bufferImageCopy.imageSubresource.baseArrayLayer = 0;
            bufferImageCopy.imageSubresource.layerCount = arrayLayers;
            bufferImageCopy.imageSubresource.mipLevel = 0;

            vkCmdCopyBufferToImage(vulkanContext.tempCommandBuffer, stagingBuffer.vkBuffer, vulkanImage.vkImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
        }
      
        cmdGenerateMipLevels(vulkanContext.tempCommandBuffer, vulkanImage.vkImage, 0, arrayLayers, mipLevels, imageCI.extent);

        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = mipLevels;

            vkCmdPipelineBarrier(vulkanContext.tempCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            vulkanImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        checkVkResult(vkEndCommandBuffer(vulkanContext.tempCommandBuffer));

        {
            VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            VkFence fence = VK_NULL_HANDLE;
            checkVkResult(vkCreateFence(vulkanContext.device, &fenceCI, nullptr, &fence));

            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &vulkanContext.tempCommandBuffer;
            checkVkResult(vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submitInfo, fence));

            checkVkResult(vkWaitForFences(vulkanContext.device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
            vkDestroyFence(vulkanContext.device, fence, nullptr);
        }

        destroyBuffer(vulkanContext, stagingBuffer);
        stbi_image_free(pRawImageData);

        return vulkanImage;
    }

    VulkanImage loadCubeImage2DFromFile(const VulkanContext &vulkanContext, const char *cubeJsonFilePath)
    {
        std::ifstream file(cubeJsonFilePath);
        assert(file.is_open());

        nlohmann::json cubeMapJson;
        file >> cubeMapJson;

        int width = 0;
        int height = 0;
        int numChannels = 0;

        auto *right = stbi_load(cubeMapJson["right"].get<std::string>().c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
        auto *left = stbi_load(cubeMapJson["left"].get<std::string>().c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
        auto *top = stbi_load(cubeMapJson["top"].get<std::string>().c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
        auto *bottom = stbi_load(cubeMapJson["bottom"].get<std::string>().c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
        auto *forward = stbi_load(cubeMapJson["forward"].get<std::string>().c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
        auto *backward = stbi_load(cubeMapJson["backward"].get<std::string>().c_str(), &width, &height, &numChannels, STBI_rgb_alpha);

        const std::size_t faceSize = 4 * width * height;

        VulkanBuffer stagingBuffer = createBuffer(vulkanContext, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                                            faceSize * 6);  
        
        std::memcpy(stagingBuffer.pMappedPointer, right, faceSize);
        std::memcpy(stagingBuffer.pMappedPointer + faceSize, left, faceSize);
        std::memcpy(stagingBuffer.pMappedPointer + 2 * faceSize, top, faceSize);
        std::memcpy(stagingBuffer.pMappedPointer + 3 * faceSize, bottom, faceSize);
        std::memcpy(stagingBuffer.pMappedPointer + 4 * faceSize, forward, faceSize);
        std::memcpy(stagingBuffer.pMappedPointer + 5 * faceSize, backward, faceSize);

        const VkFormat format = vulkanContext.findSupportedFormat({VK_FORMAT_R8G8B8A8_SRGB}, VK_IMAGE_TILING_OPTIMAL, 
                                                        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR);
        assert(format != VK_FORMAT_UNDEFINED);

        const VulkanImageDesc cubeImageDesc = VulkanImageDesc::create(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT, {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u}, 
                                                                format, 6, true);

        VulkanImage vulkanImage = createCubeImage2D(vulkanContext, cubeImageDesc);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        checkVkResult(vkBeginCommandBuffer(vulkanContext.tempCommandBuffer, &commandBufferBeginInfo));

        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = vulkanImage.vkImage;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = vulkanImage.arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = vulkanImage.mipLevels;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(vulkanContext.tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                            0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            vulkanImage.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }

        std::array<VkBufferImageCopy, 6> bufferImageCopies = {};

        for (std::size_t i = 0; i != bufferImageCopies.size(); ++i) {
            VkBufferImageCopy bufferImageCopy = {};
            bufferImageCopy.bufferOffset = i * faceSize;
            bufferImageCopy.bufferRowLength = static_cast<uint32_t>(width);
            bufferImageCopy.bufferImageHeight = static_cast<uint32_t>(height);
            bufferImageCopy.imageExtent.width = static_cast<uint32_t>(width);
            bufferImageCopy.imageExtent.height = static_cast<uint32_t>(height);
            bufferImageCopy.imageExtent.depth = 1;
            bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferImageCopy.imageSubresource.baseArrayLayer = i;
            bufferImageCopy.imageSubresource.layerCount = 1;            
            bufferImageCopy.imageSubresource.mipLevel = 0;

            bufferImageCopies[i] = bufferImageCopy;
        }

        vkCmdCopyBufferToImage(vulkanContext.tempCommandBuffer, stagingBuffer.vkBuffer, vulkanImage.vkImage, vulkanImage.imageLayout, 
                        static_cast<uint32_t>(bufferImageCopies.size()), bufferImageCopies.data());
        
        cmdGenerateMipLevels(vulkanContext.tempCommandBuffer, vulkanImage.vkImage, 0, vulkanImage.arrayLayers, vulkanImage.mipLevels, 
                            {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1});

        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = vulkanImage.vkImage;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = vulkanImage.arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = vulkanImage.mipLevels;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(vulkanContext.tempCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
                            0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            vulkanImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        checkVkResult(vkEndCommandBuffer(vulkanContext.tempCommandBuffer));

        {
            VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            VkFence fence = VK_NULL_HANDLE;
            checkVkResult(vkCreateFence(vulkanContext.device, &fenceCI, nullptr, &fence));

            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &vulkanContext.tempCommandBuffer;

            checkVkResult(vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submitInfo, fence));
            checkVkResult(vkWaitForFences(vulkanContext.device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
            vkDestroyFence(vulkanContext.device, fence, nullptr);
        }   

        destroyBuffer(vulkanContext,stagingBuffer);
        stbi_image_free(right);
        stbi_image_free(left);
        stbi_image_free(top);
        stbi_image_free(bottom);
        stbi_image_free(forward);
        stbi_image_free(backward);

        return vulkanImage;
    }

    VkImageView createImageView(const VulkanContext &vulkanContext, const VkImage image, const VkImageViewType viewType, 
                            const VkFormat format, const VkImageAspectFlags imageAspect, const uint32_t baseArrayLayer, 
                            const uint32_t layerCount, const uint32_t baseMipLevel, const uint32_t levelCount)
    {
        VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewCI.image = image;
        imageViewCI.viewType = viewType;
        imageViewCI.format = format;
        imageViewCI.subresourceRange.aspectMask = imageAspect;
        imageViewCI.subresourceRange.baseArrayLayer = baseArrayLayer;
        imageViewCI.subresourceRange.layerCount = layerCount;
        imageViewCI.subresourceRange.baseMipLevel = baseMipLevel;
        imageViewCI.subresourceRange.levelCount = levelCount;

        VkImageView imageView = VK_NULL_HANDLE;
        checkVkResult(vkCreateImageView(vulkanContext.device, &imageViewCI, nullptr, &imageView));

        return imageView;
    }

    void destroyImage(const VulkanContext &vulkanContext, VulkanImage &vulkanImage)
    {
        vmaDestroyImage(vulkanContext.vmaAllocator, vulkanImage.vkImage, vulkanImage.vmaAllocation);
        vkDestroyImageView(vulkanContext.device, vulkanImage.vkImageView, nullptr);
        vulkanImage = {};
    }
    
    void cmdGenerateMipLevels(const VkCommandBuffer commandBuffer, const VkImage vkImage, const uint32_t baseArrayLayer, const uint32_t numArrayLayers,
                    const uint32_t mipLevels, const VkExtent3D extent)
    {
        uint32_t width = extent.width;
        uint32_t height = extent.height;

        for (uint32_t i = 1; i != mipLevels; ++i) {
            if (width > 1) {
                width /= 2;
            }

            if (height > 1) {
                height /= 2;
            }

            {
                VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
                imageMemoryBarrier.image = vkImage;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageMemoryBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
                imageMemoryBarrier.subresourceRange.layerCount = numArrayLayers;
                imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
                imageMemoryBarrier.subresourceRange.levelCount = 1;
                imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                            0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }

            VkImageCopy imageCopy = {};
            imageCopy.extent.width = width;
            imageCopy.extent.height = height;
            imageCopy.extent.depth = 1;
            
            imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.srcSubresource.baseArrayLayer = baseArrayLayer;
            imageCopy.srcSubresource.layerCount = numArrayLayers;
            imageCopy.srcSubresource.mipLevel = i - 1;

            imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.dstSubresource.baseArrayLayer = baseArrayLayer;
            imageCopy.dstSubresource.layerCount = numArrayLayers;
            imageCopy.dstSubresource.mipLevel = i;

            vkCmdCopyImage(commandBuffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
        }

        // The last miplevel from baseArraylayer to numArrayLayers - 1 did not get a chance to have VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout
        // The reason is the caller should expect all the baseArrayLayer to numArrayLayers - 1 miplevels image layout to be in  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = vkImage;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
            imageMemoryBarrier.subresourceRange.layerCount = numArrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
            imageMemoryBarrier.subresourceRange.levelCount = 1;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                        0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }  
    }     
}