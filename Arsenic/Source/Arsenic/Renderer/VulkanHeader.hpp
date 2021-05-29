#pragma once

#include "Arsenic/Core/Logger.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace arsenic
{
	inline void checkVkResult(const VkResult result)
	{
		switch (result) {
		case VK_SUCCESS:
			break;
		case VK_NOT_READY:
			ARSENIC_WARN("checkVkResult A fence or query has not yet completed");
			assert(false);
			break;
		case VK_TIMEOUT:
			ARSENIC_WARN("checkVkResult A wait operation has not completed in the specified time");
			assert(false);
			break;
		case VK_EVENT_SET:
			ARSENIC_WARN("checkVkResult An event is signaled");
			assert(false);
			break;
		case VK_EVENT_RESET:
			ARSENIC_WARN("checkVkResult An event is unsignaled");
			assert(false);
			break;
		case VK_INCOMPLETE:
			ARSENIC_WARN("checkVkResult A ARSENIC_WARN(array was too small for the result");
			assert(false);		
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			ARSENIC_WARN("checkVkResult A host memory allocation has failed");
			assert(false);			
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			ARSENIC_WARN("checkVkResult A device memory allocation has failed");
			assert(false);		
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			ARSENIC_WARN("checkVkResult Initialization of an object could not be completed for implementation-specific reasons");
			assert(false);
			break;
		case VK_ERROR_DEVICE_LOST:
			ARSENIC_WARN("checkVkResult The logical or physical device has been lost");
			assert(false);
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			ARSENIC_WARN("checkVkResult Mapping of a memory object has failed");
			assert(false);
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			ARSENIC_WARN("checkVkResult A requested layer is not present or could not be loaded");
			assert(false);
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			ARSENIC_WARN("checkVkResult A requested extension is not supported");
			assert(false);
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			ARSENIC_WARN("checkVkResult A requested feature is not supported");
			assert(false);
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			ARSENIC_WARN("checkVkResult The requested version of Vulkan is not supported by the driver or is otherwise incompatible");
			assert(false);
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			ARSENIC_WARN("checkVkResult Too many objects of the type have already been created");
			assert(false);
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			ARSENIC_WARN("checkVkResult A requested format is not supported on this device");
			assert(false);
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			ARSENIC_WARN("checkVkResult A surface is no longer available");
			assert(false);
			break;
		case VK_SUBOPTIMAL_KHR:
			ARSENIC_WARN("checkVkResult A swapchain no longer matches the surface properties exactly, but can still be used");
			assert(false);
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			ARSENIC_WARN("checkVkResult A surface has changed in such a way that it is no longer compatible with the swapchain");
			assert(false);
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			ARSENIC_WARN("checkVkResult The display used by a swapchain does not use the same presentable image layout");
			assert(false);
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			ARSENIC_WARN("checkVkResult The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API");
			assert(false);
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			ARSENIC_WARN("checkVkResult A validation layer found an error");
			assert(false);
			break;
		default:
			ARSENIC_WARN("checkVkResult ERROR: UNKNOWN VULKAN ERROR");
			assert(false);
			break;
		}
    }	

}