#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Renderer/PhysicalDevice.hpp"

namespace arsenic
{
    static bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const char* const* ppPhysicalDeviceExtensions, 
													const std::size_t physicalDeviceExtensionCount)
	{
		uint32_t extensionCount = 0;
		checkVkResult(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));

		assert(extensionCount != 0);
	
		std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
		checkVkResult(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, supportedExtensions.data()));

		for (std::size_t i = 0; i != physicalDeviceExtensionCount; ++i) {
			bool extensionFound = false;

			for (const auto& extension : supportedExtensions) {
				if (std::strcmp(ppPhysicalDeviceExtensions[i], extension.extensionName) == 0) {
					extensionFound = true;
				}
			}

			if (!extensionFound) {
				return false;
			}
		}
	
		return true;
	}

	static QueueFamilies findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
		assert(count);

		std::vector<VkQueueFamilyProperties> supportedQueueFamilies(count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, supportedQueueFamilies.data());

		QueueFamilies queueFamilies;

		uint32_t i = 0;
		for (const auto &queueFamily : supportedQueueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				queueFamilies.graphicsFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				queueFamilies.computeFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				queueFamilies.transferFamily = i;
			}

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport) {
				queueFamilies.presentFamily = i;
			}

			if (queueFamilies.isComplete()) {
				break;
			}
		}

		return queueFamilies;
	}

	static uint32_t scoreVkPhysicalDevice(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties props = {};
		VkPhysicalDeviceFeatures features = {};
		vkGetPhysicalDeviceProperties(physicalDevice, &props);
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);

		uint32_t score = 0;

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 100;
		}
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			score += 10;
		}

		return score;
	}

	static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const char* const* ppPhysicalDeviceExtensions, const std::size_t physicalDeviceExtensionCount)
	{
		const bool extensionSupported = checkPhysicalDeviceExtensionSupport(physicalDevice, ppPhysicalDeviceExtensions, physicalDeviceExtensionCount);
		const QueueFamilies queueFamiles = findQueueFamilies(physicalDevice, surface);

		return extensionSupported && queueFamiles.isComplete();
	}
	
	PhysicalDevice PhysicalDevice::chooseOptimalPhysicalDevice(const Instance& vulkanInstance, VkSurfaceKHR surfaceVk,
                                                 const char* const* ppPhysicalDeviceExtensions, const std::size_t physicalDeviceExtensionCount)
	{
		uint32_t gpuCount = 0;
		checkVkResult(vkEnumeratePhysicalDevices(vulkanInstance.vkinstance, &gpuCount, nullptr));

		assert(gpuCount != 0);
	
		std::vector<::VkPhysicalDevice> physicalDevices(gpuCount);
		checkVkResult(vkEnumeratePhysicalDevices(vulkanInstance.vkinstance, &gpuCount, physicalDevices.data()));

		std::multimap<uint32_t, VkPhysicalDevice> deviceCandiates;

		for (auto physicalDevice : physicalDevices) {
			if (isDeviceSuitable(physicalDevice, surfaceVk, ppPhysicalDeviceExtensions, physicalDeviceExtensionCount)) {
				const auto score = scoreVkPhysicalDevice(physicalDevice);
				deviceCandiates.insert({score, physicalDevice});	
			}
		}

		if (deviceCandiates.rbegin()->first != 0) {
			PhysicalDevice selectedPhysicalDevice = {};
			selectedPhysicalDevice.vkPhysicalDevice = deviceCandiates.rbegin()->second,
			selectedPhysicalDevice.queueFamilies = findQueueFamilies(selectedPhysicalDevice.vkPhysicalDevice, surfaceVk);
			
			vkGetPhysicalDeviceProperties(selectedPhysicalDevice.vkPhysicalDevice, &selectedPhysicalDevice.deviceProperties);
			vkGetPhysicalDeviceFeatures(selectedPhysicalDevice.vkPhysicalDevice, &selectedPhysicalDevice.deviceFeatures);
			vkGetPhysicalDeviceMemoryProperties(selectedPhysicalDevice.vkPhysicalDevice, &selectedPhysicalDevice.memoryProperties);
		
			return selectedPhysicalDevice;
		}

		ARSENIC_CRITICAL("No suitable graphics card available for this application");
		assert(false);
		return {};
	}

    std::optional<uint32_t> PhysicalDevice::findSuitableMemoryTypeIndex(const PhysicalDevice& vulkanPhysicalDevice, const uint32_t typeBits, const VkMemoryPropertyFlags memoryPropertyFlags)
    {
	    for (uint32_t i = 0; i != vulkanPhysicalDevice.memoryProperties.memoryTypeCount; ++i) {
	        if ((typeBits & (1 << i)) && ((vulkanPhysicalDevice.memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)) {
	            return i;
	        }
	    }

	    return std::optional<uint32_t >();
    }

	void PhysicalDevice::logVulkanPhysicalDevice(const PhysicalDevice &vulkanPhysicalDevice)
	{
		uint32_t extensionCount = 0;
		checkVkResult(vkEnumerateDeviceExtensionProperties(vulkanPhysicalDevice.vkPhysicalDevice, nullptr, &extensionCount, nullptr));

		std::vector<VkExtensionProperties> supportedExtensionProperties(extensionCount);

		checkVkResult(vkEnumerateDeviceExtensionProperties(vulkanPhysicalDevice.vkPhysicalDevice, nullptr, &extensionCount, supportedExtensionProperties.data()));

		std::stringstream ss;
		switch (static_cast<int32_t>(vulkanPhysicalDevice.deviceProperties.deviceType)) {
		case 1:
			ss << "Integrated";
			break;
		case 2:
			ss << "Discrete";
			break;
		case 3:
			ss << "Virtual";
			break;
		case 4:
			ss << "CPU";
			break;
		default:
			ss << "Other " << vulkanPhysicalDevice.deviceProperties.deviceType;
		}

		ss << " Physical Device: " << vulkanPhysicalDevice.deviceProperties.deviceID;
		switch (vulkanPhysicalDevice.deviceProperties.vendorID) {
		case 0x8086:
			ss << " \"Intel\"";
			break;
		case 0x10DE:
			ss << " \"Nvidia\"";
			break;
		case 0x1002:
			ss << " \"AMD\"";
			break;
		default:
			ss << " \"" << vulkanPhysicalDevice.deviceProperties.vendorID << '\"';
		}

		ss << ' ' << vulkanPhysicalDevice.deviceProperties.deviceName << ' ';

		const std::array<uint32_t, 3> supportedVersion = {
			VK_VERSION_MAJOR(vulkanPhysicalDevice.deviceProperties.apiVersion),
			VK_VERSION_MINOR(vulkanPhysicalDevice.deviceProperties.apiVersion),
			VK_VERSION_PATCH(vulkanPhysicalDevice.deviceProperties.apiVersion)
		};

		ss << "API Version: " << supportedVersion[0] << "." << supportedVersion[1] << "." << supportedVersion[2];
		ARSENIC_INFO(ss.str());
	}
}