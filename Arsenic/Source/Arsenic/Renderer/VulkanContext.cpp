#include "Arsenic/Arsenicpch.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Arsenic/Renderer/VulkanHeader.hpp"
#include "Arsenic/Renderer/VulkanContext.hpp"
#include "Arsenic/Renderer/Structure.hpp"

#include "nlohmann/json.hpp"
#include "stb_image.hpp"

namespace arsenic
{	
    constexpr std::array<const char*, 2> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE3_EXTENSION_NAME,
       // VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        //VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        //VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        //VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        //mkVK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    };

	static VkSurfaceKHR createSurfaceKHR(GLFWwindow* pGLFWwindow, const Instance &instance)
	{
		VkSurfaceKHR surfaceVk = VK_NULL_HANDLE;
		checkVkResult(glfwCreateWindowSurface(instance.vkinstance, pGLFWwindow, nullptr, &surfaceVk));

		return surfaceVk;
	}

	static void destroySurfaceKHR(VkSurfaceKHR surface, const Instance& instance)
	{
		vkDestroySurfaceKHR(instance.vkinstance, surface, nullptr);
	}

    
	VulkanContext createVulkanContext(GLFWwindow *pGLfWwindow, const std::string& appName)
	{	
        VulkanContext vulkanContext = {};
        vulkanContext.instance = Instance::createVulkanInstance(appName);

        checkVkResult(glfwCreateWindowSurface(vulkanContext.instance.vkinstance, pGLfWwindow, nullptr, &vulkanContext.surface));

		vulkanContext.physicalDevice = PhysicalDevice::chooseOptimalPhysicalDevice(vulkanContext.instance, vulkanContext.surface, deviceExtensions.data(), 
                                                                            static_cast<uint32_t>(deviceExtensions.size()));
        
        const std::set<uint32_t> uniqueQueueFamilies = { 
            *vulkanContext.physicalDevice.queueFamilies.graphicsFamily, 
            *vulkanContext.physicalDevice.queueFamilies.presentFamily,
            *vulkanContext.physicalDevice.queueFamilies.transferFamily
        };

        std::vector<VkDeviceQueueCreateInfo> deviceQueueCIS;
        deviceQueueCIS.reserve(uniqueQueueFamilies.size());

        constexpr float queuePriority = 1.0f;

        for (const auto queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCI = {};
            queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCI.queueFamilyIndex = queueFamily;
            queueCI.queueCount = 1;
            queueCI.pQueuePriorities = &queuePriority;

            deviceQueueCIS.emplace_back(queueCI);
        }
        
        VkPhysicalDeviceFeatures features = {};
        features.fillModeNonSolid = true;

        VkDeviceCreateInfo deviceCreateCI = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceCreateCI.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCIS.size());
        deviceCreateCI.pQueueCreateInfos = deviceQueueCIS.data();
        deviceCreateCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateCI.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateCI.pEnabledFeatures = &features;

        checkVkResult(vkCreateDevice(vulkanContext.physicalDevice.vkPhysicalDevice, &deviceCreateCI, nullptr, &vulkanContext.device));

        vkGetDeviceQueue(vulkanContext.device, *vulkanContext.physicalDevice.queueFamilies.graphicsFamily, 0, &vulkanContext.graphicsQueue);
        vkGetDeviceQueue(vulkanContext.device, *vulkanContext.physicalDevice.queueFamilies.presentFamily, 0, &vulkanContext.presentQueue);
        vkGetDeviceQueue(vulkanContext.device, *vulkanContext.physicalDevice.queueFamilies.transferFamily, 0, &vulkanContext.transferQueue);    

        VmaAllocatorCreateInfo vmaAllocatorCI = {};
        vmaAllocatorCI.device = vulkanContext.device;
        vmaAllocatorCI.frameInUseCount = maxFrameInFlight;
        vmaAllocatorCI.instance = vulkanContext.instance.vkinstance;
        vmaAllocatorCI.physicalDevice = vulkanContext.physicalDevice.vkPhysicalDevice;
        vmaAllocatorCI.vulkanApiVersion = VK_API_VERSION_1_2;
        checkVkResult(vmaCreateAllocator(&vmaAllocatorCI, &vulkanContext.vmaAllocator));

        VkCommandPoolCreateInfo commandPoolCI = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        commandPoolCI.queueFamilyIndex = vulkanContext.physicalDevice.queueFamilies.graphicsFamily.value();
        commandPoolCI.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        checkVkResult(vkCreateCommandPool(vulkanContext.device, &commandPoolCI, nullptr, &vulkanContext.tempCommandPool));

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.commandPool = vulkanContext.tempCommandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        checkVkResult(vkAllocateCommandBuffers(vulkanContext.device, &commandBufferAllocateInfo, &vulkanContext.tempCommandBuffer));


        return std::move(vulkanContext);
	}

	void destroyVulkanContext(VulkanContext &vulkanContext)
	{
        checkVkResult(vkDeviceWaitIdle(vulkanContext.device));

        vkDestroyCommandPool(vulkanContext.device, vulkanContext.tempCommandPool, nullptr);
        vmaDestroyAllocator(vulkanContext.vmaAllocator);
		vkDestroySurfaceKHR(vulkanContext.instance.vkinstance, vulkanContext.surface, nullptr);
        vkDestroyDevice(vulkanContext.device, nullptr);

		Instance::destroyVulkanInstance(&vulkanContext.instance);

        vulkanContext = {};
	}

    VulkanBuffer VulkanContext::createBuffer(const BufferDesc &bufferDesc, const void *pData)
    {
        assert(bufferDesc.usage != BufferUsage::None || bufferDesc.type != BufferType::None || bufferDesc.size != 0);

        VkBufferCreateInfo bufferCI = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCI.size = bufferDesc.size;
        bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;


        switch (bufferDesc.type) {
        case BufferType::Vertex:
            bufferCI.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferType::Index:
            bufferCI.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case BufferType::Uniform:
            bufferCI.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case BufferType::Storage:
            bufferCI.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        };

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        vmaAllocationCI.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VulkanBuffer stagingBuffer = {};
        stagingBuffer.size = bufferDesc.size;
        
        VmaAllocationInfo vmaAllocationInfo = {};
        checkVkResult(vmaCreateBuffer(vmaAllocator, &bufferCI, &vmaAllocationCI, &stagingBuffer.vkBuffer, &stagingBuffer.vmaAllocation, &vmaAllocationInfo));

        stagingBuffer.pMappedPointer = static_cast<uint8_t*>(vmaAllocationInfo.pMappedData);

        if (pData != nullptr) {
            std::memcpy(stagingBuffer.pMappedPointer, pData, bufferDesc.size);
        }

        if (bufferDesc.usage == BufferUsage::Static) {
            vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaAllocationCI.flags = {};

            VulkanBuffer devicalLocalBuffer = {};
            devicalLocalBuffer.size = bufferDesc.size;       

            checkVkResult(vmaCreateBuffer(vmaAllocator, &bufferCI, &vmaAllocationCI, &devicalLocalBuffer.vkBuffer, &devicalLocalBuffer.vmaAllocation, nullptr));

            if (pData != nullptr) {
                copyBufferToBufferAndSubmit(stagingBuffer.vkBuffer, devicalLocalBuffer.vkBuffer, 0, 0, bufferDesc.size);
            }

            vmaDestroyBuffer(vmaAllocator, stagingBuffer.vkBuffer, stagingBuffer.vmaAllocation);
         
            return devicalLocalBuffer;
        }

        return stagingBuffer;
    }

    void VulkanContext::destroyBuffer(VulkanBuffer &vulkanBuffer)
    {
        assert(vulkanBuffer.vkBuffer);
        vmaDestroyBuffer(vmaAllocator, vulkanBuffer.vkBuffer, vulkanBuffer.vmaAllocation);
    }

    void VulkanContext::copyBufferToBufferAndSubmit(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize srcOffset, 
                        const VkDeviceSize dstOffset, const VkDeviceSize size)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        checkVkResult(vkBeginCommandBuffer(tempCommandBuffer, &commandBufferBeginInfo));

        VkBufferCopy bufferCopy = {};
        bufferCopy.srcOffset = srcOffset;
        bufferCopy.dstOffset= dstOffset;
        bufferCopy.size = size;

        vkCmdCopyBuffer(tempCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

        checkVkResult(vkEndCommandBuffer(tempCommandBuffer));

        VkSubmitInfo submtInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submtInfo.commandBufferCount = 1;
        submtInfo.pCommandBuffers = &tempCommandBuffer;

        VkFence fence = VK_NULL_HANDLE;

        VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        checkVkResult(vkCreateFence(device, &fenceCI, nullptr, &fence));

        checkVkResult(vkQueueSubmit(graphicsQueue, 1, &submtInfo, fence));
        checkVkResult(vkWaitForFences(device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));

        vkDestroyFence(device, fence, nullptr);
    }


    VulkanImage VulkanContext::createImage2D(const TextureFormat format, const char *filePath)
    {
        assert(format != TextureFormat::None || format != TextureFormat::RGBAF32 || format != TextureFormat::RGBAF32SRGB);

        RawTextureInfo rawTexture = loadRawTexture(filePath);

        VkFormat vkformat = VK_FORMAT_UNDEFINED;

        switch (format) {
        case TextureFormat::RGBAU8:
            vkformat = findSupportedFormat({VK_FORMAT_R8G8B8A8_UNORM}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
            break;
        case TextureFormat::RGBAU8SRGB:
            vkformat = findSupportedFormat({VK_FORMAT_R8G8B8A8_SRGB}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
            break;
        }

        uint32_t mipLevels = calculateMipLevels(rawTexture.width, rawTexture.height);

        VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.extent.width = rawTexture.width;
        imageCI.extent.height = rawTexture.height;
        imageCI.extent.depth = 1;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCI.format = vkformat;
        imageCI.arrayLayers = 1;
        imageCI.mipLevels = mipLevels;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        
        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VulkanImage vulkanImage = {};
        checkVkResult(vmaCreateImage(vmaAllocator, &imageCI, &vmaAllocationCI, &vulkanImage.vkImage, &vulkanImage.vmaAllocation, nullptr));

        BufferDesc bufferDesc = {};
        bufferDesc.size = rawTexture.numChannels * rawTexture.width * rawTexture.height;
        bufferDesc.usage = BufferUsage::Dynamic;
        bufferDesc.type = BufferType::Buffer;

        VulkanBuffer stagingBuffer = createBuffer(bufferDesc, rawTexture.pRawTextureData);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        checkVkResult(vkBeginCommandBuffer(tempCommandBuffer, &commandBufferBeginInfo));

        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = vulkanImage.vkImage;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = imageCI.arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = imageCI.mipLevels;
            
            vkCmdPipelineBarrier(tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }
                
        VkBufferImageCopy bufferImageCopy = {};
        bufferImageCopy.imageExtent.width = rawTexture.width;
        bufferImageCopy.imageExtent.height = rawTexture.height;
        bufferImageCopy.imageExtent.depth = 1.0f;
        bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferImageCopy.imageSubresource.baseArrayLayer = 0;
        bufferImageCopy.imageSubresource.layerCount = imageCI.arrayLayers;
        bufferImageCopy.imageSubresource.mipLevel = 0;

        vkCmdCopyBufferToImage(tempCommandBuffer, stagingBuffer.vkBuffer, vulkanImage.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

        cmdGenerateMipLevels(tempCommandBuffer, vulkanImage.vkImage, mipLevels, imageCI.arrayLayers, {rawTexture.width, rawTexture.height, 1});

        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = vulkanImage.vkImage;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = imageCI.arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = imageCI.mipLevels;
            
            vkCmdPipelineBarrier(tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            
            vulkanImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        
        checkVkResult(vkEndCommandBuffer(tempCommandBuffer));

        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &tempCommandBuffer;

        VkFence fence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        checkVkResult(vkCreateFence(device, &fenceCI, nullptr, &fence));
        
        checkVkResult(vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence));
        checkVkResult(vkWaitForFences(device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));

        vkDestroyFence(device, fence, nullptr);
        destroyBuffer(stagingBuffer);
        freeRawTexture(rawTexture.pRawTextureData);

        VkImageViewCreateInfo imageviewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageviewCI.image = vulkanImage.vkImage;
        imageviewCI.format = vkformat;
        imageviewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageviewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageviewCI.subresourceRange.baseArrayLayer = 0;
        imageviewCI.subresourceRange.layerCount = imageCI.arrayLayers;
        imageviewCI.subresourceRange.baseMipLevel = 0;
        imageviewCI.subresourceRange.levelCount = imageCI.mipLevels;

        checkVkResult(vkCreateImageView(device, &imageviewCI, nullptr, &vulkanImage.vkImageView));

        return vulkanImage;
    }
    
    VulkanImage VulkanContext::createImage2D(const TextureFormat format, const VkExtent2D extent, const uint32_t arrayLayers, const uint32_t mipLevel)
    {
        
    }

    VulkanImage VulkanContext::createImageCube(const TextureFormat format, const char *cubeMapJsonFilePath)
    {
        assert(format != TextureFormat::None || format != TextureFormat::RGBAF32 || format != TextureFormat::RGBAF32SRGB);

        std::ifstream file(cubeMapJsonFilePath);
        assert(file.is_open());

        nlohmann::json cubeMapJson;
        file >> cubeMapJson;

        constexpr std::size_t arrayLayers = 6;

        std::array<RawTextureInfo, arrayLayers> cubeMapFaces;
        cubeMapFaces[0] = loadRawTexture(cubeMapJson["right"].get<std::string>().c_str());
        cubeMapFaces[1] = loadRawTexture(cubeMapJson["left"].get<std::string>().c_str());
        cubeMapFaces[2] = loadRawTexture(cubeMapJson["top"].get<std::string>().c_str());
        cubeMapFaces[3] = loadRawTexture(cubeMapJson["bottom"].get<std::string>().c_str());  
        cubeMapFaces[4] = loadRawTexture(cubeMapJson["forward"].get<std::string>().c_str());
        cubeMapFaces[5] = loadRawTexture(cubeMapJson["backward"].get<std::string>().c_str());

        const uint32_t width = cubeMapFaces[0].width;
        const uint32_t height = cubeMapFaces[0].height;
       // const uint32_t mipLevel = calculateMipLevels(width, height);

        VkFormat vkFormat = VK_FORMAT_UNDEFINED;

        switch (format) {
        case TextureFormat::RGBAU8:
            vkFormat = findSupportedFormat({VK_FORMAT_R8G8B8A8_UNORM}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
            break;
        case TextureFormat::RGBAU8SRGB:
            vkFormat = findSupportedFormat({VK_FORMAT_R8G8B8A8_SRGB}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
            break;
        }

        VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCI.extent = {width, height, 1};
        imageCI.format = vkFormat;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = arrayLayers;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VulkanImage vulkanImage = {};
        vulkanImage.width = width;
        vulkanImage.height = height;
        vulkanImage.arrayLayers = arrayLayers;
        vulkanImage.mipLevels = 1;
        vulkanImage.vkFormat = vkFormat;
        
        checkVkResult(vmaCreateImage(vmaAllocator, &imageCI, &vmaAllocationCI, &vulkanImage.vkImage, &vulkanImage.vmaAllocation, nullptr));       

        const std::size_t cubeMapFaceSize = 4 * width * height;

        BufferDesc bufferDesc = {};
        bufferDesc.size = cubeMapFaceSize * arrayLayers;
        bufferDesc.usage = BufferUsage::Dynamic;
        bufferDesc.type = BufferType::Buffer;

        VulkanBuffer vulkanBuffer = createBuffer(bufferDesc);

        std::array<VkBufferImageCopy, arrayLayers> bufferImageCopies = {};
        
        for (std::size_t i = 0; i != arrayLayers; ++i) {
            RawTextureInfo &rawTextureInfo = cubeMapFaces[i];
            std::memcpy(vulkanBuffer.pMappedPointer + (i * cubeMapFaceSize), rawTextureInfo.pRawTextureData, cubeMapFaceSize);
            freeRawTexture(rawTextureInfo.pRawTextureData);

            bufferImageCopies[i].bufferOffset = i * cubeMapFaceSize;
            bufferImageCopies[i].imageExtent = {width, height, 1};
            bufferImageCopies[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferImageCopies[i].imageSubresource.baseArrayLayer = i;
            bufferImageCopies[i].imageSubresource.layerCount = 1;
            bufferImageCopies[i].imageSubresource.mipLevel = 0;
        }

        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        checkVkResult(vkBeginCommandBuffer(tempCommandBuffer, &commandBufferBeginInfo));

        {
            VkImageMemoryBarrier memoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            memoryBarrier.image = vulkanImage.vkImage;
            memoryBarrier.srcAccessMask = 0;
            memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            memoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            memoryBarrier.subresourceRange.baseArrayLayer = 0;
            memoryBarrier.subresourceRange.layerCount = arrayLayers;
            memoryBarrier.subresourceRange.baseMipLevel = 0;
            memoryBarrier.subresourceRange.levelCount = 1;

            vkCmdPipelineBarrier(tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
        }

        vkCmdCopyBufferToImage(tempCommandBuffer, vulkanBuffer.vkBuffer, vulkanImage.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    static_cast<uint32_t>(bufferImageCopies.size()), bufferImageCopies.data());

        {
            VkImageMemoryBarrier memoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            memoryBarrier.image = vulkanImage.vkImage;
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            memoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            memoryBarrier.subresourceRange.baseArrayLayer = 0;
            memoryBarrier.subresourceRange.layerCount = arrayLayers;
            memoryBarrier.subresourceRange.baseMipLevel = 0;
            memoryBarrier.subresourceRange.levelCount = 1;

            vkCmdPipelineBarrier(tempCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

            vulkanImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        
        checkVkResult(vkEndCommandBuffer(tempCommandBuffer));

        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &tempCommandBuffer;
        
        VkFence fence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};   
        checkVkResult(vkCreateFence(device, &fenceCI, nullptr, &fence));

        checkVkResult(vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence));
        checkVkResult(vkWaitForFences(device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        
        vkDestroyFence(device, fence, nullptr);
        destroyBuffer(vulkanBuffer);

        VkImageViewCreateInfo imageviewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageviewCI.image = vulkanImage.vkImage;
        imageviewCI.format = vkFormat;
        imageviewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        imageviewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageviewCI.subresourceRange.baseArrayLayer = 0;
        imageviewCI.subresourceRange.layerCount = arrayLayers;
        imageviewCI.subresourceRange.baseMipLevel = 0;
        imageviewCI.subresourceRange.levelCount = 1;

        checkVkResult(vkCreateImageView(device, &imageviewCI, nullptr, &vulkanImage.vkImageView));
 
        return vulkanImage;
    }
    
    void VulkanContext::destroyImage(VulkanImage &vulkanImage)
    {
        vmaDestroyImage(vmaAllocator, vulkanImage.vkImage, vulkanImage.vmaAllocation);
        vkDestroyImageView(device, vulkanImage.vkImageView, nullptr);
    }

    VkFormat VulkanContext::findSupportedFormat(const std::initializer_list<VkFormat> &candidates, const VkImageTiling tiling, 
                                    const VkFormatFeatureFlags features) const
    {
        for (const VkFormat format : candidates) {
            VkFormatProperties formatProperties = {};
            vkGetPhysicalDeviceFormatProperties(physicalDevice.vkPhysicalDevice, format, &formatProperties);

            if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        ARSENIC_WARN("Renderer: Unable to find a format that supports the required features with the desire tiling");
        return {};
    }

    //  All mipLevels image layout needs to be in transfer dst optimal
    void VulkanContext::cmdGenerateMipLevels(const VkCommandBuffer commandBuffer, const VkImage vkImage, const uint32_t mipLevels, 
                        const uint32_t arrayLayers, const VkExtent3D extent3D)
    {
         uint32_t width = extent3D.width;
        uint32_t height = extent3D.height;

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
                imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
                imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;
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
            imageCopy.srcSubresource.baseArrayLayer = 0;
            imageCopy.srcSubresource.layerCount = arrayLayers;
            imageCopy.srcSubresource.mipLevel = i - 1;

            imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.dstSubresource.baseArrayLayer = 0;
            imageCopy.dstSubresource.layerCount = arrayLayers;
            imageCopy.dstSubresource.mipLevel = i;

            vkCmdCopyImage(commandBuffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
        }

        // last miplevel in all the layers did not get a chance to have VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout
        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = vkImage;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;
            imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
            imageMemoryBarrier.subresourceRange.levelCount = 1;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                        0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }  
    }     

    
    RawTextureInfo VulkanContext::loadRawTexture(const char *filePath) const
    {
        int width = 0;
        int height = 0;
        int numChannels = 0;

        stbi_uc *pImageData = stbi_load(filePath, &width, &height, &numChannels, STBI_rgb_alpha);
        assert(pImageData);

        RawTextureInfo info = {};
        info.width = static_cast<uint32_t>(width);
        info.height = static_cast<uint32_t>(height);
        info.numChannels = 4u;
        info.pRawTextureData = pImageData;

        return info;
    }

    void VulkanContext::freeRawTexture(uint8_t *pRawTextureData) const
    {
        assert(pRawTextureData);
        stbi_image_free(pRawTextureData);
    }  
}
