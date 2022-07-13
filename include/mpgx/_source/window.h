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

#define VK_VERSION VK_API_VERSION_1_2
#define VK_FRAME_LAG 2

#if MPGX_SUPPORT_VULKAN
typedef struct VkWindow_T
{
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;
	uint32_t transferQueueFamilyIndex;
	uint32_t computeQueueFamilyIndex;
	VkDevice device;
	VmaAllocator allocator;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkQueue computeQueue;
	VkCommandPool graphicsCommandPool;
	VkCommandPool presentCommandPool;
	VkCommandPool transferCommandPool;
	VkCommandPool computeCommandPool;
	VkCommandBuffer transferCommandBuffer;
	VkCommandBuffer computeCommandBuffer;
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
	VkPhysicalDeviceProperties deviceProperties;
	bool isDeviceIntegrated;
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
	if (callbackData->messageIdNumber == 0x6bbb14)
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

	fprintf(stderr,
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
		return vkToMpgxResult(vkResult);

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
		return vkToMpgxResult(vkResult);
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
		return vkToMpgxResult(vkResult);

	VkExtensionProperties* extensionProperties = malloc(
		extensionPropertyCount * sizeof(VkExtensionProperties));

	if (!extensionProperties)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	vkResult = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionPropertyCount,
		extensionProperties);

	if (vkResult != VK_SUCCESS)
	{
		free(extensionProperties);
		return vkToMpgxResult(vkResult);
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
	const char* engineName,
	uint8_t engineVersionMajor,
	uint8_t engineVersionMinor,
	uint8_t engineVersionPatch,
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
	assert(engineName);
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
		engineVersionMajor,
		engineVersionMinor,
		engineVersionPatch);
	VkApplicationInfo applicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		NULL,
		appName,
		appVersion,
		engineName,
		engineVersion,
		VK_VERSION,
	};

#if __APPLE__
	VkInstanceCreateFlags flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
	VkInstanceCreateFlags flags = 0;
#endif

	VkInstanceCreateInfo instanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		NULL,
		flags,
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
		return vkToMpgxResult(vkResult);

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
		return vkToMpgxResult(vkResult);

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
	VkPhysicalDevice* physicalDevice,
	VkPhysicalDeviceProperties* deviceProperties,
	bool* isIntegrated)
{
	assert(instance);
	assert(physicalDevice);
	assert(deviceProperties);
	assert(isIntegrated);

	uint32_t deviceCount;

	VkResult vkResult = vkEnumeratePhysicalDevices(
		instance,
		&deviceCount,
		NULL);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

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
		return vkToMpgxResult(vkResult);
	}

	uint64_t targetDeviceIndex = 0;
	uint64_t targetScore = 0;

	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;

	if (deviceCount > 1)
	{
		for (uint32_t i = 0; i < deviceCount; i++)
		{
			device = devices[i];

			vkGetPhysicalDeviceProperties(
				device,
				&properties);

			uint64_t score;

			if (properties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				score = 100000;
			}
			else if (properties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
			{
				score = 90000;
			}
			else if (properties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				score = 80000;
			}
			else if (properties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_CPU)
			{
				score = 70000;
			}
			else
			{
				score = 0;
			}

			score += properties.limits.maxImageDimension2D;

			// TODO: add other tests

			if (score > targetScore)
			{
				targetDeviceIndex = i;
				targetScore = score;
			}
		}
	}

	device = devices[targetDeviceIndex];
	free(devices);

	vkGetPhysicalDeviceProperties(
		device,
		&properties);

	*physicalDevice = device;
	*deviceProperties = properties;
	*isIntegrated = properties.deviceType ==
		VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult getVkQueueFamilyIndices(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	uint32_t* graphicsQueueFamilyIndex,
	uint32_t* presentQueueFamilyIndex,
	uint32_t* transferQueueFamilyIndex,
	uint32_t* computeQueueFamilyIndex)
{
	assert(physicalDevice);
	assert(surface);
	assert(graphicsQueueFamilyIndex);
	assert(presentQueueFamilyIndex);
	assert(transferQueueFamilyIndex);
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
		transferIndex = UINT32_MAX,
		computeIndex = UINT32_MAX;

	for (uint32_t i = 0; i < propertyCount; i++)
	{
		VkQueueFamilyProperties* property = &properties[i];

		if (property->queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsIndex = i;
			break;
		}
	}

	if (graphicsIndex == UINT32_MAX) // TODO: possibly support processing only
	{
		free(properties);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < propertyCount; i++)
	{
		VkBool32 isSupported;

		VkResult vkResult = vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice,
			i,
			surface,
			&isSupported);

		if (vkResult != VK_SUCCESS)
		{
			free(properties);
			return vkToMpgxResult(vkResult);
		}

		if (isSupported == VK_TRUE)
		{
			presentIndex = i;
			break;
		}
	}

	if (presentIndex == UINT32_MAX)
	{
		free(properties);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < propertyCount; i++)
	{
		VkQueueFamilyProperties* property = &properties[i];

		if (property->queueFlags & VK_QUEUE_TRANSFER_BIT &&
			graphicsIndex != i)
		{
			transferIndex = i;
			break;
		}
	}

	if (transferIndex == UINT32_MAX)
	{
		for (uint32_t i = 0; i < propertyCount; i++)
		{
			VkQueueFamilyProperties* property = &properties[i];

			if (property->queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				transferIndex = i;
				break;
			}
		}

		if (transferIndex == UINT32_MAX)
		{
			free(properties);
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		}
	}

	for (uint32_t i = 0; i < propertyCount; i++)
	{
		VkQueueFamilyProperties* property = &properties[i];

		if (property->queueFlags & VK_QUEUE_COMPUTE_BIT &&
			graphicsIndex != i && transferIndex != i)
		{
			computeIndex = i;
			break;
		}
	}

	if (computeIndex == UINT32_MAX)
	{
		for (uint32_t i = 0; i < propertyCount; i++)
		{
			VkQueueFamilyProperties* property = &properties[i];

			if (property->queueFlags & VK_QUEUE_COMPUTE_BIT &&
				graphicsIndex != i)
			{
				computeIndex = i;
				break;
			}
		}

		if (computeIndex == UINT32_MAX)
		{
			for (uint32_t i = 0; i < propertyCount; i++)
			{
				VkQueueFamilyProperties* property = &properties[i];

				if (property->queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					computeIndex = i;
					break;
				}
			}

			if (computeIndex == UINT32_MAX)
			{
				free(properties);
				return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
			}
		}
	}

	*graphicsQueueFamilyIndex = graphicsIndex;
	*presentQueueFamilyIndex = presentIndex;
	*transferQueueFamilyIndex = transferIndex;
	*computeQueueFamilyIndex = computeIndex;

	free(properties);
	return SUCCESS_MPGX_RESULT;
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
		return vkToMpgxResult(vkResult);

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
		return vkToMpgxResult(vkResult);
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
	uint32_t transferQueueFamilyIndex,
	uint32_t computeQueueFamilyIndex,
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

	VkDeviceQueueCreateInfo queueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		NULL,
		0,
		0,
		1,
		&priority,
	};

	VkDeviceQueueCreateInfo queueCreateInfos[4];
	uint32_t queueCreateInfoCount = 0;

	queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	queueCreateInfos[queueCreateInfoCount++] = queueCreateInfo;

	if (presentQueueFamilyIndex != graphicsQueueFamilyIndex)
	{
		queueCreateInfo.queueFamilyIndex = presentQueueFamilyIndex;
		queueCreateInfos[queueCreateInfoCount++] = queueCreateInfo;
	}
	if (transferQueueFamilyIndex != graphicsQueueFamilyIndex &&
		transferQueueFamilyIndex != presentQueueFamilyIndex)
	{
		queueCreateInfo.queueFamilyIndex = transferQueueFamilyIndex;
		queueCreateInfos[queueCreateInfoCount++] = queueCreateInfo;
	}
	if (computeQueueFamilyIndex != graphicsQueueFamilyIndex &&
		computeQueueFamilyIndex != presentQueueFamilyIndex &&
		computeQueueFamilyIndex != transferQueueFamilyIndex)
	{
		queueCreateInfo.queueFamilyIndex = computeQueueFamilyIndex;
		queueCreateInfos[queueCreateInfoCount++] = queueCreateInfo;
	}

	VkPhysicalDeviceFeatures2 features;
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features.pNext = NULL;

	vkGetPhysicalDeviceFeatures2(
		physicalDevice,
		&features);

#if __APPLE__
	VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilitySubsetFeatures;
	memset(&portabilitySubsetFeatures, 0,
		sizeof(VkPhysicalDevicePortabilitySubsetFeaturesKHR));
	features.pNext = &portabilitySubsetFeatures;
	portabilitySubsetFeatures.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;
	portabilitySubsetFeatures.mutableComparisonSamplers = VK_TRUE;
#endif

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

#if __APPLE__
		portabilitySubsetFeatures.pNext = &bufferDeviceAddressFeatures;
#else
		features.pNext = &bufferDeviceAddressFeatures;
#endif
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
		&features,
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
		return vkToMpgxResult(vkResult);

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
		return vkToMpgxResult(vkResult);

	*vmaAllocator = vmaAllocatorInstance;
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
	bool useBeginClear,
	bool useDeferredShading,
	bool useRayTracing,
	Vec2I framebufferSize,
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
		return vkToMpgxResult(vkResult);
	}

	window->surface = surface;

	bool isDeviceIntegrated;

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties deviceProperties;

	MpgxResult mpgxResult = getBestVkPhysicalDevice(
		instance,
		&physicalDevice,
		&deviceProperties,
		&isDeviceIntegrated);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->physicalDevice = physicalDevice;
	window->deviceProperties = deviceProperties;
	window->isDeviceIntegrated = isDeviceIntegrated;

	uint32_t graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		transferQueueFamilyIndex,
		computeQueueFamilyIndex;

	mpgxResult = getVkQueueFamilyIndices(
		physicalDevice,
		surface,
		&graphicsQueueFamilyIndex,
		&presentQueueFamilyIndex,
		&transferQueueFamilyIndex,
		&computeQueueFamilyIndex);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkWindow(instance, window);
		return mpgxResult;
	}

	window->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	window->presentQueueFamilyIndex = presentQueueFamilyIndex;
	window->transferQueueFamilyIndex = transferQueueFamilyIndex;
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
		if (!isExtensionSupported[deferredHostOperationsExtIndex] ||
			!isExtensionSupported[accelerationStructureExtIndex] ||
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
		transferQueueFamilyIndex,
		computeQueueFamilyIndex,
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
		transferQueue,
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
		transferQueueFamilyIndex,
		0,
		&transferQueue);
	vkGetDeviceQueue(
		device,
		computeQueueFamilyIndex,
		0,
		&computeQueue);

	window->graphicsQueue = graphicsQueue;
	window->presentQueue = presentQueue;
	window->transferQueue = transferQueue;
	window->computeQueue = computeQueue;

	VkCommandPoolCreateInfo commandPoolCreateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		graphicsQueueFamilyIndex
	};

	VkCommandPool graphicsCommandPool;

	vkResult = vkCreateCommandPool(
		device,
		&commandPoolCreateInfo,
		NULL,
		&graphicsCommandPool);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);
		return vkToMpgxResult(vkResult);
	}

	window->graphicsCommandPool = graphicsCommandPool;

	VkCommandPool presentCommandPool;

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		presentCommandPool = graphicsCommandPool;
	}
	else
	{
		commandPoolCreateInfo.queueFamilyIndex =
			presentQueueFamilyIndex;

		vkResult = vkCreateCommandPool(
			device,
			&commandPoolCreateInfo,
			NULL,
			&presentCommandPool);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkWindow(instance, window);
			return vkToMpgxResult(vkResult);
		}
	}

	window->presentCommandPool = presentCommandPool;

	commandPoolCreateInfo.flags |=
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCreateInfo.queueFamilyIndex =
		transferQueueFamilyIndex;

	VkCommandPool transferCommandPool;

	vkResult = vkCreateCommandPool(
		device,
		&commandPoolCreateInfo,
		NULL,
		&transferCommandPool);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);
		return vkToMpgxResult(vkResult);
	}

	window->transferCommandPool = transferCommandPool;

	commandPoolCreateInfo.queueFamilyIndex =
		computeQueueFamilyIndex;

	VkCommandPool computeCommandPool;

	vkResult = vkCreateCommandPool(
		device,
		&commandPoolCreateInfo,
		NULL,
		&computeCommandPool);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);
		return vkToMpgxResult(vkResult);
	}

	window->computeCommandPool = computeCommandPool;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		transferCommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1,
	};

	VkCommandBuffer transferCommandBuffer;

	vkResult = vkAllocateCommandBuffers(
		device,
		&commandBufferAllocateInfo,
		&transferCommandBuffer);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);
		return vkToMpgxResult(vkResult);
	}

	window->transferCommandBuffer = transferCommandBuffer;

	commandBufferAllocateInfo.commandPool =
		computeCommandPool;

	VkCommandBuffer computeCommandBuffer;

	vkResult = vkAllocateCommandBuffers(
		device,
		&commandBufferAllocateInfo,
		&computeCommandBuffer);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);
		return vkToMpgxResult(vkResult);
	}

	window->computeCommandBuffer = computeCommandBuffer;

	VkFence* fences = window->fences;
	VkSemaphore* imageAcquiredSemaphores =
		window->imageAcquiredSemaphores;
	VkSemaphore* drawCompleteSemaphores =
		window->drawCompleteSemaphores;
	VkSemaphore* imageOwnershipSemaphores =
		window->imageOwnershipSemaphores;

	VkFenceCreateInfo fenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		NULL,
		VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (uint8_t i = 0; i < VK_FRAME_LAG; i++)
	{
		VkFence fence;

		vkResult = vkCreateFence(
			device,
			&fenceCreateInfo,
			NULL,
			&fence);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkWindow(instance, window);
			return vkToMpgxResult(vkResult);
		}

		fences[i] = fence;

		VkSemaphoreCreateInfo semaphoreCreateInfo = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			NULL,
			0,
		};

		VkSemaphore semaphore;

		vkResult = vkCreateSemaphore(
			device,
			&semaphoreCreateInfo,
			NULL,
			&semaphore);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkWindow(instance, window);
			return vkToMpgxResult(vkResult);
		}

		imageAcquiredSemaphores[i] = semaphore;

		vkResult = vkCreateSemaphore(
			device,
			&semaphoreCreateInfo,
			NULL,
			&semaphore);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkWindow(instance, window);
			return vkToMpgxResult(vkResult);
		}

		drawCompleteSemaphores[i] = semaphore;

		vkResult = vkCreateSemaphore(
			device,
			&semaphoreCreateInfo,
			NULL,
			&semaphore);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkWindow(instance, window);
			return vkToMpgxResult(vkResult);
		}

		imageOwnershipSemaphores[i] = semaphore;
	}

	fenceCreateInfo.flags = 0;

	VkFence transferFence;

	vkResult = vkCreateFence(
		device,
		&fenceCreateInfo,
		NULL,
		&transferFence);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);
		return vkToMpgxResult(vkResult);
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
		useBeginClear,
		useDeferredShading,
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
