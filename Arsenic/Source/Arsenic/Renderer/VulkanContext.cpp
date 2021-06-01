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

}
