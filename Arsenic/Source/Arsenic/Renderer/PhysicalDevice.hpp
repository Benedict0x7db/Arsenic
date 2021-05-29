#pragma once

#include "Arsenic/Renderer/VulkanHeader.hpp"
#include "Arsenic/Renderer/Instance.hpp"

namespace arsenic
{
    struct QueueFamilies
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> transferFamily;
		std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;

		bool isComplete() const { return graphicsFamily.has_value() && transferFamily.has_value() && presentFamily.has_value(); }
	};

    struct PhysicalDevice
    {
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        QueueFamilies queueFamilies;

        VkPhysicalDeviceMemoryProperties memoryProperties = {};
        VkPhysicalDeviceProperties deviceProperties = {};
        VkPhysicalDeviceFeatures deviceFeatures = {};
        

        static PhysicalDevice chooseOptimalPhysicalDevice(const Instance &vulkanInstance, VkSurfaceKHR surface,
                                            const char *const* ppPhysicalDeviceExtensions, const std::size_t physicalDeviceExtensionCount);

        static void logVulkanPhysicalDevice(const PhysicalDevice& physicalDevice);

        static std::optional<uint32_t> findSuitableMemoryTypeIndex(const PhysicalDevice& physicalDevice, const uint32_t typeBits, 
                                                        const VkMemoryPropertyFlags memoryPropertyFlags);
    };
}