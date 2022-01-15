// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "mpgx/defines.h"

#if MPGX_SUPPORT_VULKAN
#include "mpgx/_source/vulkan.h"
#define GLFW_INCLUDE_VULKAN
#endif

#if MPGX_SUPPORT_OPENGL
#include "mpgx/_source/opengl.h"
#else
#define GLFW_INCLUDE_NONE
#endif

#include "mpgx/_source/swapchain.h"

#include "GLFW/glfw3.h"
#include <stdio.h>

#define ENGINE_NAME "MPGX"
#define VK_VERSION VK_API_VERSION_1_2
#define VK_FRAME_LAG 2

#if MPGX_SUPPORT_VULKAN
typedef struct VkWindow_T
{
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	bool isDeviceIntegrated;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;
	uint32_t computeQueueFamilyIndex;
	VkDevice device;
	VmaAllocator allocator;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue computeQueue;
	VkCommandPool graphicsCommandPool;
	VkCommandPool presentCommandPool;
	VkCommandPool transferCommandPool;
	VkCommandPool computeCommandPool;
	VkCommandBuffer transferCommandBuffer;
	VkFence fences[VK_FRAME_LAG];
	VkSemaphore imageAcquiredSemaphores[VK_FRAME_LAG];
	VkSemaphore drawCompleteSemaphores[VK_FRAME_LAG];
	VkSemaphore imageOwnershipSemaphores[VK_FRAME_LAG];
	VkFence transferFence;
	VkSwapchain swapchain;
	uint32_t frameIndex;
	uint32_t bufferIndex;
	VkCommandBuffer currenCommandBuffer;
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	size_t stagingSize;
} VkWindow_T;

typedef VkWindow_T* VkWindow;

static VkBool32 VKAPI_CALL vkDebugMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
	assert(callbackData);

#if __APPLE__
	if (callbackData->messageIdNumber == 0x6bbb14 ||
		callbackData->messageIdNumber == 0xf467460 ||
		strcmp(callbackData->pMessage, "Unrecognized "
		"CreateInstance->pCreateInfo->pApplicationInfo->apiVersion "
		"number (0x00402000). Assuming MoltenVK 1.1 version.") == 0)
	{
		// TODO: fix MoltenVK Vulkan 1.2 shader error logs
		return VK_FALSE;
	}
#endif

	const char* severity;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		severity = "Vulkan VERBOSE";
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		severity = "Vulkan INFO";
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		severity = "Vulkan WARNING";
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		severity = "Vulkan ERROR";

	fprintf(stdout,
		"%s: %s\n",
		severity,
		callbackData->pMessage);
	return VK_FALSE;
}

inline static MpgxResult checkVkInstanceLayers(
	const char** layers,
	bool* isLayerSupported,
	uint32_t layerCount)
{
	assert(layers);
	assert(isLayerSupported);
	assert(layerCount > 0);

	uint32_t layerPropertyCount;

	VkResult vkResult = vkEnumerateInstanceLayerProperties(
		&layerPropertyCount,
		NULL);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	VkLayerProperties* layerProperties = malloc(
		layerPropertyCount * sizeof(VkLayerProperties));

	if (!layerProperties)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	vkResult = vkEnumerateInstanceLayerProperties(
		&layerPropertyCount,
		layerProperties);

	if (vkResult != VK_SUCCESS)
	{
		free(layerProperties);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < layerCount; i++)
	{
		bool isFound = false;

		for (uint32_t j = 0; j < layerPropertyCount; j++)
		{
			if (strcmp(layers[i],
				layerProperties[j].layerName) == 0)
			{
				isFound = true;
				break;
			}
		}

		isLayerSupported[i] = isFound;
	}

	free(layerProperties);
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult checkVkInstanceExtensions(
	const char** extensions,
	bool* isExtensionSupported,
	uint32_t extensionCount)
{
	assert(extensions);
	assert(isExtensionSupported);
	assert(extensionCount > 0);

	uint32_t extensionPropertyCount;

	VkResult vkResult = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionPropertyCount,
		NULL);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	VkExtensionProperties* extensionProperties = malloc(
		extensionPropertyCount * sizeof(VkLayerProperties));

	if (!extensionProperties)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	vkResult = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionPropertyCount,
		extensionProperties);

	if (vkResult != VK_SUCCESS)
	{
		free(extensionProperties);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_LAYER_NOT_PRESENT)
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < extensionCount; i++)
	{
		bool isFound = false;

		for (uint32_t j = 0; j < extensionPropertyCount; j++)
		{
			if (strcmp(extensions[i],
				extensionProperties[j].extensionName) == 0)
			{
				isFound = true;
				break;
			}
		}

		isExtensionSupported[i] = isFound;
	}

	free(extensionProperties);
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkInstance(
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch,
	const char** layers,
	uint32_t layerCount,
	const char** extensions,
	uint32_t extensionCount,
	VkInstance* instance)
{
	assert(appName);
	assert(layers);
	assert(layerCount > 0);
	assert(extensions);
	assert(extensionCount > 0);
	assert(instance);

	uint32_t appVersion = VK_MAKE_API_VERSION(0,
		appVersionMajor,
		appVersionMinor,
		appVersionPatch);
	const uint32_t engineVersion = VK_MAKE_API_VERSION(0,
		MPGX_VERSION_MAJOR,
		MPGX_VERSION_MINOR,
		MPGX_VERSION_PATCH);
	VkApplicationInfo applicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		NULL,
		appName,
		appVersion,
		ENGINE_NAME,
		engineVersion,
		VK_VERSION,
	};

	VkInstanceCreateInfo instanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		NULL,
		0,
		&applicationInfo,
		layerCount,
		layers,
		extensionCount,
		extensions,
	};

#ifndef NDEBUG
	for (uint32_t i = 0; i < extensionCount; i++)
	{
		if (strcmp(extensions[i],
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
		{
			VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {
				VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				NULL,
				0,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
				vkDebugMessengerCallback,
				NULL,
			};

			instanceCreateInfo.pNext = &messengerCreateInfo;
			break;
		}
	}
#endif

	VkInstance vkInstance;

	VkResult vkResult = vkCreateInstance(
		&instanceCreateInfo,
		NULL,
		&vkInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_INITIALIZATION_FAILED)
			return FAILED_TO_INITIALIZE_VULKAN_MPGX_RESULT;
		else if (vkResult == VK_ERROR_LAYER_NOT_PRESENT ||
			vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*instance = vkInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVkDebugUtilsMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT* debugUtilsMessenger)
{
	assert(instance);
	assert(debugUtilsMessenger);

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		NULL,
		0,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
		vkDebugMessengerCallback,
		NULL,
	};

	PFN_vkCreateDebugUtilsMessengerEXT createFunction =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");

	if (!createFunction)
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;

	VkDebugUtilsMessengerEXT debugUtilsMessengerInstance;

	VkResult vkResult = createFunction(
		instance,
		&createInfo,
		NULL,
		&debugUtilsMessengerInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*debugUtilsMessenger = debugUtilsMessengerInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroyVkDebugUtilsMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugUtilsMessenger)
{
	assert(instance);

	if (!debugUtilsMessenger)
		return;

	PFN_vkDestroyDebugUtilsMessengerEXT destroyFunction =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");

	if (!destroyFunction)
		abort();

	destroyFunction(
		instance,
		debugUtilsMessenger,
		NULL);
}

inline static MpgxResult getBestVkPhysicalDevice(
	VkInstance instance,
	bool* isIntegrated,
	VkPhysicalDevice* physicalDevice)
{
	assert(instance);
	assert(isIntegrated);
	assert(physicalDevice);

	uint32_t deviceCount;

	VkResult vkResult = vkEnumeratePhysicalDevices(
		instance,
		&deviceCount,
		NULL);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_INITIALIZATION_FAILED)
			return FAILED_TO_INITIALIZE_VULKAN_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	VkPhysicalDevice* devices = malloc(
		deviceCount * sizeof(VkPhysicalDevice));

	if (!devices)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	vkResult = vkEnumeratePhysicalDevices(
		instance,
		&deviceCount,
		devices);

	if (vkResult != VK_SUCCESS)
	{
		free(devices);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_INITIALIZATION_FAILED)
			return FAILED_TO_INITIALIZE_VULKAN_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	VkPhysicalDevice targetDevice = NULL;
	uint32_t targetScore = 0;
	bool integrated = false;

	for (uint32_t i = 0; i < deviceCount; i++)
	{
		VkPhysicalDevice device = devices[i];
		VkPhysicalDeviceProperties properties;

		vkGetPhysicalDeviceProperties(
			device,
			&properties);

		uint32_t score = 0;

		if (properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}
		else if (properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		{
			score += 750;
		}
		else if (properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			score += 500;
		}
		else if (properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_CPU)
		{
			score += 250;
		}

		// TODO: add other tests

		if (score > targetScore)
		{
			targetDevice = device;
			targetScore = score;

			integrated = properties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		}
	}

	free(devices);

	*isIntegrated = integrated;
	*physicalDevice = targetDevice;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult getVkQueueFamilyIndices(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	uint32_t* graphicsQueueFamilyIndex,
	uint32_t* presentQueueFamilyIndex,
	uint32_t* computeQueueFamilyIndex)
{
	assert(physicalDevice);
	assert(surface);
	assert(graphicsQueueFamilyIndex);
	assert(presentQueueFamilyIndex);
	assert(computeQueueFamilyIndex);

	uint32_t propertyCount;

	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&propertyCount,
		NULL);

	VkQueueFamilyProperties* properties = malloc(
		propertyCount * sizeof(VkQueueFamilyProperties));

	if (!properties)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&propertyCount,
		properties);

	uint32_t graphicsIndex = UINT32_MAX,
		presentIndex = UINT32_MAX,
		computeIndex = UINT32_MAX;

	for (uint32_t i = 0; i < propertyCount; i++)
	{
		VkQueueFamilyProperties* property = &properties[i];

		if (property->queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (graphicsIndex == UINT32_MAX)
				graphicsIndex = i;
		}
		if (property->queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			if (computeIndex == UINT32_MAX)
				computeIndex = i;
		}

		VkBool32 isSupported;

		VkResult vkResult = vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice,
			i,
			surface,
			&isSupported);

		// TESTING PURPOSE:
		//if (graphicsIndex == i) continue;

		if (vkResult != VK_SUCCESS)
		{
			free(properties);

			if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
				return OUT_OF_HOST_MEMORY_MPGX_RESULT;
			else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
			else if (vkResult == VK_ERROR_SURFACE_LOST_KHR)
				return SURFACE_IS_LOST_MPGX_RESULT;
			else
				return UNKNOWN_ERROR_MPGX_RESULT;
		}

		if (isSupported == VK_TRUE)
		{
			if (presentIndex == UINT32_MAX)
				presentIndex = i;
		}

		if (graphicsIndex != UINT32_MAX &&
			presentIndex != UINT32_MAX &&
			computeIndex != UINT32_MAX)
		{
			*graphicsQueueFamilyIndex = graphicsIndex;
			*presentQueueFamilyIndex = presentIndex;
			*computeQueueFamilyIndex = computeIndex;

			free(properties);
			return SUCCESS_MPGX_RESULT;
		}
	}

	free(properties);
	return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
}

inline static MpgxResult checkVkDeviceExtensions(
	VkPhysicalDevice physicalDevice,
	const char** extensions,
	bool* isExtensionSupported,
	uint32_t extensionCount)
{
	assert(physicalDevice);
	assert(extensions);
	assert(isExtensionSupported);
	assert(extensionCount > 0);

	uint32_t propertyCount;

	VkResult vkResult = vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		NULL,
		&propertyCount,
		NULL);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_LAYER_NOT_PRESENT)
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	VkExtensionProperties* properties = malloc(
		propertyCount * sizeof(VkExtensionProperties));

	if (!properties)
		return false;

	vkResult = vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		NULL,
		&propertyCount,
		properties);

	if (vkResult != VK_SUCCESS)
	{
		free(properties);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_LAYER_NOT_PRESENT)
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < extensionCount; i++)
	{
		bool isFound = false;

		for (uint32_t j = 0; j < propertyCount; j++)
		{
			if (strcmp(extensions[i],
				properties[j].extensionName) == 0)
			{
				isFound = true;
				break;
			}
		}

		isExtensionSupported[i] = isFound;
	}

	free(properties);
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkDevice(
	VkPhysicalDevice physicalDevice,
	uint32_t graphicsQueueFamilyIndex,
	uint32_t presentQueueFamilyIndex,
	bool useRayTracing,
	const char** extensions,
	uint32_t extensionCount,
	VkDevice* device)
{
	assert(physicalDevice);
	assert(extensions);
	assert(extensionCount > 0);
	assert(device);

	float priority = 1.0f;

	VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		NULL,
		0,
		graphicsQueueFamilyIndex,
		1,
		&priority,
	};
	VkDeviceQueueCreateInfo presentQueueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		NULL,
		0,
		presentQueueFamilyIndex,
		1,
		&priority,
	};
	const VkDeviceQueueCreateInfo oneCreateInfos[1] = {
		graphicsQueueCreateInfo,
	};
	const VkDeviceQueueCreateInfo twoCreateInfos[2] = {
		graphicsQueueCreateInfo,
		presentQueueCreateInfo,
	};

	const VkDeviceQueueCreateInfo* queueCreateInfos;
	uint32_t queueCreateInfoCount;

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		queueCreateInfos = oneCreateInfos;
		queueCreateInfoCount = 1;
	}
	else
	{
		queueCreateInfos = twoCreateInfos;
		queueCreateInfoCount = 2;
	}

	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures;
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures;

	if (useRayTracing)
	{
		memset(&bufferDeviceAddressFeatures, 0,
			sizeof(VkPhysicalDeviceBufferDeviceAddressFeatures));
		memset(&accelerationStructureFeatures, 0,
			sizeof(VkPhysicalDeviceAccelerationStructureFeaturesKHR));
		memset(&rayTracingPipelineFeatures, 0,
			sizeof(VkPhysicalDeviceRayTracingPipelineFeaturesKHR));

		bufferDeviceAddressFeatures.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
		bufferDeviceAddressFeatures.pNext = &accelerationStructureFeatures;
		accelerationStructureFeatures.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		accelerationStructureFeatures.accelerationStructure = VK_TRUE;
		accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
		rayTracingPipelineFeatures.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
	}

	VkDeviceCreateInfo deviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		useRayTracing ? &bufferDeviceAddressFeatures : NULL,
		0,
		queueCreateInfoCount,
		queueCreateInfos,
		0,
		NULL,
		extensionCount,
		extensions,
		NULL,
	};

	VkResult vkResult = vkCreateDevice(
		physicalDevice,
		&deviceCreateInfo,
		NULL,
		device);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_INITIALIZATION_FAILED)
			return FAILED_TO_INITIALIZE_VULKAN_MPGX_RESULT;
		else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT ||
			vkResult == VK_ERROR_FEATURE_NOT_PRESENT)
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		else if (vkResult == VK_ERROR_DEVICE_LOST)
			return DEVICE_IS_LOST_MPGX_RESULT; // TODO: handle VK_ERROR_TOO_MANY_OBJECTS
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVmaAllocator(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkInstance instance,
	bool hasMemoryBudgetExt,
	bool hasDeviceAddressExt,
	VmaAllocator* vmaAllocator)
{
	assert(physicalDevice);
	assert(device);
	assert(instance);
	assert(vmaAllocator);

	VmaAllocatorCreateInfo createInfo;

	memset(&createInfo,
		0, sizeof(VmaAllocatorCreateInfo));

	createInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
	createInfo.physicalDevice = physicalDevice;
	createInfo.device = device;
	createInfo.instance = instance;
	createInfo.vulkanApiVersion = VK_VERSION;

	if (hasMemoryBudgetExt)
		createInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	if (hasDeviceAddressExt)
		createInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	VmaAllocator vmaAllocatorInstance;

	VkResult vkResult = vmaCreateAllocator(
		&createInfo,
		&vmaAllocatorInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*vmaAllocator = vmaAllocatorInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVkCommandPool(
	VkDevice device,
	VkCommandPoolCreateFlags flags,
	uint32_t queueFamilyIndex,
	VkCommandPool* commandPool)
{
	assert(device);
	assert(commandPool);

	VkCommandPoolCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		flags,
		queueFamilyIndex,
	};

	VkCommandPool commandPoolInstance;

	VkResult vkResult = vkCreateCommandPool(
		device,
		&createInfo,
		NULL,
		&commandPoolInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*commandPool = commandPoolInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult allocateVkCommandBuffer(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandBuffer* commandBuffer)
{
	assert(device);
	assert(commandPool);
	assert(commandBuffer);

	VkCommandBufferAllocateInfo allocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1,
	};

	VkCommandBuffer commandBufferInstance;

	VkResult vkResult = vkAllocateCommandBuffers(
		device,
		&allocateInfo,
		&commandBufferInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*commandBuffer = commandBufferInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkWindow(
	VkInstance instance,
	VkWindow window)
{
	assert(instance);

	if (!window)
		return;

	VkDevice device = window->device;
	VmaAllocator allocator = window->allocator;

	if (allocator)
	{
		vmaDestroyBuffer(
			allocator,
			window->stagingBuffer,
			window->stagingAllocation);

		VkCommandPool graphicsCommandPool = window->graphicsCommandPool;
		VkCommandPool presentCommandPool = window->presentCommandPool;

		if (device)
		{
			destroyVkSwapchain(
				device,
				allocator,
				graphicsCommandPool,
				presentCommandPool,
				window->swapchain);

			vkDestroyFence(
				device,
				window->transferFence,
				NULL);

			VkFence* fences = window->fences;
			VkSemaphore* imageAcquiredSemaphores =
				window->imageAcquiredSemaphores;
			VkSemaphore* drawCompleteSemaphores =
				window->drawCompleteSemaphores;
			VkSemaphore* imageOwnershipSemaphores =
				window->imageOwnershipSemaphores;

			for (uint8_t i = 0; i < VK_FRAME_LAG; i++)
			{
				vkDestroySemaphore(
					device,
					imageOwnershipSemaphores[i],
					NULL);
				vkDestroySemaphore(
					device,
					drawCompleteSemaphores[i],
					NULL);
				vkDestroySemaphore(
					device,
					imageAcquiredSemaphores[i],
					NULL);

				VkFence fence = fences[i];

				if (fence)
				{
					VkResult vkResult = vkWaitForFences(
						device,
						1,
						&fence,
						VK_TRUE,
						UINT64_MAX);

					if (vkResult != VK_SUCCESS)
						abort();

					vkDestroyFence(
						device,
						fences[i],
						NULL);
				}
			}

			vkFreeCommandBuffers(
				device,
				window->transferCommandPool,
				1,
				&window->transferCommandBuffer);
			vkDestroyCommandPool(
				device,
				window->computeCommandPool,
				NULL);
			vkDestroyCommandPool(
				device,
				window->transferCommandPool,
				NULL);

			if (window->graphicsQueueFamilyIndex ==
				window->presentQueueFamilyIndex)
			{
				vkDestroyCommandPool(
					device,
					graphicsCommandPool,
					NULL);
			}
			else
			{
				vkDestroyCommandPool(
					device,
					presentCommandPool,
					NULL);
				vkDestroyCommandPool(
					device,
					graphicsCommandPool,
					NULL);
			}
		}

		vmaDestroyAllocator(allocator);
	}

	vkDestroyDevice(
		device,
		NULL);
	vkDestroySurfaceKHR(
		instance,
		window->surface,
		NULL);
	free(window);
}
inline static MpgxResult createVkWindow(
	VkInstance instance,
	GLFWwindow* handle,
	bool useStencilBuffer,
	bool useRayTracing,
	Vec2U framebufferSize,
	VkWindow* vkWindow)
{
	assert(instance);
	assert(handle);
	assert(framebufferSize.x > 0);
	assert(framebufferSize.y > 0);
	assert(vkWindow);

	VkWindow window = calloc(1, sizeof(VkWindow_T));

	if (!window)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkSurfaceKHR surface;

	VkResult vkResult = glfwCreateWindowSurface(
		instance,
		handle,
		NULL,
		&surface);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	window->surface = surface;

	bool isDeviceIntegrated;

	VkPhysicalDevice physicalDevice;

	MpgxResult mpgxResult = getBestVkPhysicalDevice(
		instance,
		&isDeviceIntegrated,
		&physicalDevice);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->physicalDevice = physicalDevice;
	window->isDeviceIntegrated = isDeviceIntegrated;

	uint32_t graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		computeQueueFamilyIndex;

	mpgxResult = getVkQueueFamilyIndices(
		physicalDevice,
		surface,
		&graphicsQueueFamilyIndex,
		&presentQueueFamilyIndex,
		&computeQueueFamilyIndex);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	window->presentQueueFamilyIndex = presentQueueFamilyIndex;
	window->computeQueueFamilyIndex = computeQueueFamilyIndex;

	const char* extensions[6];
	const char* targetExtensions[6];
	bool isExtensionSupported[6];
	uint32_t extensionCount = 0;
	uint32_t targetExtensionCount = 0;

	extensions[extensionCount++] =
	targetExtensions[targetExtensionCount] =
		VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	uint32_t swapchainExtIndex = targetExtensionCount++;

#if __APPLE__
	extensions[extensionCount++] =
	targetExtensions[targetExtensionCount] =
		"VK_KHR_portability_subset";
	uint32_t portabilitySubsetExtIndex = targetExtensionCount++;
#endif

	uint32_t memoryBudgetExtIndex;

	targetExtensions[targetExtensionCount] =
		VK_EXT_MEMORY_BUDGET_EXTENSION_NAME;
	memoryBudgetExtIndex = targetExtensionCount++;

	uint32_t deferredHostOperationsExtIndex;
	uint32_t accelerationStructureExtIndex;
	uint32_t rayTracingPipelineExtIndex;

	if (useRayTracing)
	{
		extensions[extensionCount++] =
		targetExtensions[targetExtensionCount] =
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
		deferredHostOperationsExtIndex = targetExtensionCount++;

		extensions[extensionCount++] =
		targetExtensions[targetExtensionCount] =
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
		accelerationStructureExtIndex = targetExtensionCount++;

		extensions[extensionCount++] =
		targetExtensions[targetExtensionCount] =
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
		rayTracingPipelineExtIndex = targetExtensionCount++;
	}

	mpgxResult = checkVkDeviceExtensions(
		physicalDevice,
		targetExtensions,
		isExtensionSupported,
		targetExtensionCount);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	if (!isExtensionSupported[swapchainExtIndex])
	{
		destroyVkWindow(instance, window);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

#if __APPLE__
	if (!isExtensionSupported[portabilitySubsetExtIndex])
	{
		destroyVkWindow(instance, window);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}
#endif

	if (isExtensionSupported[memoryBudgetExtIndex])
		extensions[extensionCount++] = targetExtensions[memoryBudgetExtIndex];

	if (useRayTracing)
	{
		if (!isExtensionSupported[deferredHostOperationsExtIndex] |
			!isExtensionSupported[accelerationStructureExtIndex] |
			!isExtensionSupported[rayTracingPipelineExtIndex])
		{
			destroyVkWindow(instance, window);
			return RAY_TRACING_IS_NOT_SUPPORTED_MPGX_RESULT;
		}
	}

	VkDevice device;

	mpgxResult = createVkDevice(
		physicalDevice,
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		useRayTracing,
		extensions,
		extensionCount,
		&device);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->device = device;

	VmaAllocator allocator;

	mpgxResult = createVmaAllocator(
		physicalDevice,
		device,
		instance,
		isExtensionSupported[memoryBudgetExtIndex],
		useRayTracing,
		&allocator);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->allocator = allocator;

	VkQueue graphicsQueue,
		presentQueue,
		computeQueue;

	vkGetDeviceQueue(
		device,
		graphicsQueueFamilyIndex,
		0,
		&graphicsQueue);
	vkGetDeviceQueue(
		device,
		presentQueueFamilyIndex,
		0,
		&presentQueue);
	vkGetDeviceQueue(
		device,
		computeQueueFamilyIndex,
		0,
		&computeQueue);

	window->graphicsQueue = graphicsQueue;
	window->presentQueue = presentQueue;
	window->computeQueue = computeQueue;

	VkCommandPool graphicsCommandPool;

	mpgxResult = createVkCommandPool(
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		graphicsQueueFamilyIndex,
		&graphicsCommandPool);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->graphicsCommandPool = graphicsCommandPool;

	VkCommandPool presentCommandPool;

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		presentCommandPool = graphicsCommandPool;
	}
	else
	{
		mpgxResult = createVkCommandPool(
			device,
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			presentQueueFamilyIndex,
			&presentCommandPool);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyVkWindow(instance, window);
			return mpgxResult;
		}
	}

	window->presentCommandPool = presentCommandPool;

	VkCommandPool transferCommandPool;

	mpgxResult = createVkCommandPool(
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		graphicsQueueFamilyIndex,
		&transferCommandPool);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->transferCommandPool = transferCommandPool;

	VkCommandPool computeCommandPool;

	mpgxResult = createVkCommandPool(
		device,
		0, // TODO: should we add VK_COMMAND_POOL_CREATE_TRANSIENT_BIT?
		computeQueueFamilyIndex,
		&computeCommandPool);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->computeCommandPool = computeCommandPool;

	VkCommandBuffer transferCommandBuffer;

	mpgxResult = allocateVkCommandBuffer(
		device,
		transferCommandPool,
		&transferCommandBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->transferCommandBuffer = transferCommandBuffer;

	VkFence* fences = window->fences;
	VkSemaphore* imageAcquiredSemaphores =
		window->imageAcquiredSemaphores;
	VkSemaphore* drawCompleteSemaphores =
		window->drawCompleteSemaphores;
	VkSemaphore* imageOwnershipSemaphores =
		window->imageOwnershipSemaphores;

	for (uint8_t i = 0; i < VK_FRAME_LAG; i++)
	{
		VkFence fence;

		mpgxResult = createVkFence(
			device,
			VK_FENCE_CREATE_SIGNALED_BIT,
			&fence);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyVkWindow(instance, window);
			return mpgxResult;
		}

		fences[i] = fence;

		VkSemaphore semaphore;

		mpgxResult = createVkSemaphore(
			device,
			&semaphore);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyVkWindow(instance, window);
			return mpgxResult;
		}

		imageAcquiredSemaphores[i] = semaphore;

		mpgxResult = createVkSemaphore(
			device,
			&semaphore);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyVkWindow(instance, window);
			return mpgxResult;
		}

		drawCompleteSemaphores[i] = semaphore;

		mpgxResult = createVkSemaphore(
			device,
			&semaphore);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyVkWindow(instance, window);
			return mpgxResult;
		}

		imageOwnershipSemaphores[i] = semaphore;
	}

	VkFence transferFence;

	mpgxResult = createVkFence(
		device,
		0,
		&transferFence);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->transferFence = transferFence;

	VkSwapchain swapchain;

	mpgxResult = createVkSwapchain(
		surface,
		physicalDevice,
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		device,
		allocator,
		graphicsCommandPool,
		presentCommandPool,
		useStencilBuffer,
		framebufferSize,
		&swapchain);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->swapchain = swapchain;
	window->frameIndex = 0;
	window->bufferIndex = 0;
	window->currenCommandBuffer = NULL;
	window->stagingBuffer = NULL;
	window->stagingAllocation = NULL;
	window->stagingSize = 0;

	*vkWindow = window;
	return SUCCESS_MPGX_RESULT;
}
#endif

// TODO: implement DemoUpdateTargetIPD (VK_GOOGLE) from cube.c
// TODO: investigate if we should use (demo_flush_init_cmd) from cube.c