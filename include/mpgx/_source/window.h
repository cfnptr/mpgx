// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
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
	bool isGpuIntegrated;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;
	VkDevice device;
	VmaAllocator allocator;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkCommandPool graphicsCommandPool;
	VkCommandPool presentCommandPool;
	VkCommandPool transferCommandPool;
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

inline static bool checkVkInstanceLayers(
	const char** layers,
	bool* isLayerSupported,
	uint32_t layerCount)
{
	uint32_t layerPropertyCount;

	VkResult result = vkEnumerateInstanceLayerProperties(
		&layerPropertyCount,
		NULL);

	if (result != VK_SUCCESS || layerPropertyCount == 0)
		return false;

	VkLayerProperties* layerProperties = malloc(
		layerPropertyCount * sizeof(VkLayerProperties));

	if (layerProperties == NULL)
		return false;

	result = vkEnumerateInstanceLayerProperties(
		&layerPropertyCount,
		layerProperties);

	if (result != VK_SUCCESS || layerPropertyCount == 0)
	{
		free(layerProperties);
		return false;
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
	return true;
}
inline static bool checkVkInstanceExtensions(
	const char** extensions,
	bool* isExtensionSupported,
	uint32_t extensionCount)
{
	uint32_t extensionPropertyCount;

	VkResult result = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionPropertyCount,
		NULL);

	if (result != VK_SUCCESS || extensionPropertyCount == 0)
		return false;

	VkExtensionProperties* extensionProperties = malloc(
		extensionPropertyCount * sizeof(VkLayerProperties));

	if (extensionProperties == NULL)
		return false;

	result = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionPropertyCount,
		extensionProperties);

	if (result != VK_SUCCESS || extensionPropertyCount == 0)
	{
		free(extensionProperties);
		return false;
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
	return true;
}
inline static VkInstance createVkInstance(
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch,
	const char** layers,
	uint32_t layerCount,
	const char** extensions,
	uint32_t extensionCount)
{
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

	VkInstance instance;

	VkResult result = vkCreateInstance(
		&instanceCreateInfo,
		NULL,
		&instance);

	if (result != VK_SUCCESS)
		return NULL;

	return instance;
}

inline static VkDebugUtilsMessengerEXT createVkDebugUtilsMessenger(
	VkInstance instance)
{
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

	if (createFunction == NULL)
		return NULL;

	VkDebugUtilsMessengerEXT messenger;

	VkResult result = createFunction(
		instance,
		&createInfo,
		NULL,
		&messenger);

	if (result != VK_SUCCESS)
		return NULL;

	return messenger;
}
inline static void destroyVkDebugUtilsMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugUtilsMessenger)
{
	if (debugUtilsMessenger == NULL)
		return;

	PFN_vkDestroyDebugUtilsMessengerEXT destroyFunction =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");

	if (destroyFunction == NULL)
		abort();

	destroyFunction(
		instance,
		debugUtilsMessenger,
		NULL);
}

inline static VkPhysicalDevice getBestVkPhysicalDevice(
	VkInstance instance,
	bool* _isGpuIntegrated)
{
	uint32_t deviceCount;

	VkResult result = vkEnumeratePhysicalDevices(
		instance,
		&deviceCount,
		NULL);

	if (result != VK_SUCCESS || deviceCount == 0)
		return NULL;

	VkPhysicalDevice* devices = malloc(
		deviceCount * sizeof(VkPhysicalDevice));

	if (devices == NULL)
		return NULL;

	result = vkEnumeratePhysicalDevices(
		instance,
		&deviceCount,
		devices);

	if (result != VK_SUCCESS ||
		deviceCount == 0)
	{
		free(devices);
		return NULL;
	}

	VkPhysicalDevice targetDevice = NULL;
	uint32_t targetScore = 0;
	bool isGpuIntegrated = false;

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

			isGpuIntegrated = properties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		}
	}

	free(devices);

	*_isGpuIntegrated = isGpuIntegrated;
	return targetDevice;
}
inline static bool getVkQueueFamilyIndices(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	uint32_t* _graphicsQueueFamilyIndex,
	uint32_t* _presentQueueFamilyIndex)
{
	uint32_t propertyCount;

	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&propertyCount,
		NULL);

	if (propertyCount == 0)
		return false;

	VkQueueFamilyProperties* properties = malloc(
		propertyCount * sizeof(VkQueueFamilyProperties));

	if (properties == NULL)
		return false;

	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&propertyCount,
		properties);

	if (propertyCount == 0)
	{
		free(properties);
		return false;
	}

	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t presentQueueFamilyIndex = UINT32_MAX;

	for (uint32_t i = 0; i < propertyCount; i++)
	{
		VkQueueFamilyProperties* property = &properties[i];

		if (property->queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (graphicsQueueFamilyIndex == UINT32_MAX)
				graphicsQueueFamilyIndex = i;
		}

		VkBool32 isSupported;

		VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice,
			i,
			surface,
			&isSupported);

		// TESTING PURPOSE:
		//if (graphicsQueueFamilyIndex == i) continue;

		if (result != VK_SUCCESS)
		{
			free(properties);
			return false;
		}

		if (isSupported == VK_TRUE)
		{
			if (presentQueueFamilyIndex == UINT32_MAX)
				presentQueueFamilyIndex = i;
		}

		if (graphicsQueueFamilyIndex != UINT32_MAX &&
			presentQueueFamilyIndex != UINT32_MAX)
		{
			*_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
			*_presentQueueFamilyIndex = presentQueueFamilyIndex;

			free(properties);
			return true;
		}
	}

	free(properties);
	return false;
}

inline static bool checkVkDeviceExtensions(
	VkPhysicalDevice physicalDevice,
	const char** extensions,
	bool* isExtensionSupported,
	uint32_t extensionCount)
{
	uint32_t propertyCount;

	VkResult result = vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		NULL,
		&propertyCount,
		NULL);

	if (result != VK_SUCCESS || propertyCount == 0)
		return false;

	VkExtensionProperties* properties = malloc(
		propertyCount * sizeof(VkExtensionProperties));

	if (properties == NULL)
		return false;

	result = vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		NULL,
		&propertyCount,
		properties);

	if (result != VK_SUCCESS || propertyCount == 0)
	{
		free(properties);
		return false;
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
	return true;
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

	if (useRayTracing == true)
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
		useRayTracing == true ?
		&bufferDeviceAddressFeatures : NULL,
		0,
		queueCreateInfoCount,
		queueCreateInfos,
		0,
		NULL,
		extensionCount,
		extensions,
		NULL,
	};

	VkResult result = vkCreateDevice(
		physicalDevice,
		&deviceCreateInfo,
		NULL,
		device);

	if (result != VK_SUCCESS)
		return FAILED_TO_ALLOCATE_MPGX_RESULT;

	return SUCCESS_MPGX_RESULT;
}

inline static VmaAllocator createVmaAllocator(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkInstance instance,
	bool hasMemoryBudgetExt,
	bool hasDeviceAddressExt)
{
	VmaAllocatorCreateInfo createInfo;

	memset(&createInfo,
		0, sizeof(VmaAllocatorCreateInfo));

	createInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
	createInfo.physicalDevice = physicalDevice;
	createInfo.device = device;
	createInfo.instance = instance;
	createInfo.vulkanApiVersion = VK_VERSION;

	if (hasMemoryBudgetExt == true)
		createInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	if (hasDeviceAddressExt == true)
		createInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	VmaAllocator allocator;

	VkResult result = vmaCreateAllocator(
		&createInfo,
		&allocator);

	if (result != VK_SUCCESS)
		return NULL;

	return allocator;
}

inline static VkCommandPool createVkCommandPool(
	VkDevice device,
	VkCommandPoolCreateFlags flags,
	uint32_t queueFamilyIndex)
{
	VkCommandPoolCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		flags,
		queueFamilyIndex,
	};

	VkCommandPool commandPool;

	VkResult result = vkCreateCommandPool(
		device,
		&createInfo,
		NULL,
		&commandPool);

	if (result != VK_SUCCESS)
		return NULL;

	return commandPool;
}

inline static VkFence createVkFence(
	VkDevice device,
	VkFenceCreateFlags flags)
{
	VkFenceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		NULL,
		flags,
	};

	VkFence fence;

	VkResult result = vkCreateFence(
		device,
		&createInfo,
		NULL,
		&fence);

	if (result != VK_SUCCESS)
		return NULL;

	return fence;
}
inline static VkSemaphore createVkSemaphore(VkDevice device)
{
	VkSemaphoreCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		NULL,
		0,
	};

	VkSemaphore semaphore;

	VkResult result = vkCreateSemaphore(
		device,
		&createInfo,
		NULL,
		&semaphore);

	if (result != VK_SUCCESS)
		return NULL;

	return semaphore;
}

inline static void destroyVkWindow(
	VkInstance instance,
	VkWindow window)
{
	if (window == NULL)
		return;

	VkDevice device = window->device;
	VmaAllocator allocator = window->allocator;

	if (allocator != NULL)
	{
		vmaDestroyBuffer(
			allocator,
			window->stagingBuffer,
			window->stagingAllocation);

		VkCommandPool graphicsCommandPool =
			window->graphicsCommandPool;
		VkCommandPool presentCommandPool =
			window->presentCommandPool;

		if (device != NULL)
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

				if (fence != NULL)
				{
					VkResult result = vkWaitForFences(
						device,
						1,
						&fence,
						VK_TRUE,
						UINT64_MAX);

					if (result != VK_SUCCESS)
						abort();

					vkDestroyFence(
						device,
						fences[i],
						NULL);
				}
			}

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
	VkWindow window = calloc(1, sizeof(VkWindow_T));

	if (window == NULL)
		return FAILED_TO_ALLOCATE_MPGX_RESULT;

	VkSurfaceKHR surface;

	VkResult vkResult = glfwCreateWindowSurface(
		instance,
		handle,
		NULL,
		&surface);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkWindow(instance, window);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	window->surface = surface;

	bool isGpuIntegrated;

	VkPhysicalDevice physicalDevice = getBestVkPhysicalDevice(
		instance,
		&isGpuIntegrated);

	if (physicalDevice == NULL)
	{
		destroyVkWindow(instance, window);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	window->physicalDevice = physicalDevice;
	window->isGpuIntegrated = isGpuIntegrated;

	uint32_t graphicsQueueFamilyIndex,
		presentQueueFamilyIndex;

	bool result = getVkQueueFamilyIndices(
		physicalDevice,
		surface,
		&graphicsQueueFamilyIndex,
		&presentQueueFamilyIndex);

	if (result == false)
	{
		destroyVkWindow(instance, window);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	window->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	window->presentQueueFamilyIndex = presentQueueFamilyIndex;

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

	if (useRayTracing == true)
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

	result = checkVkDeviceExtensions(
		physicalDevice,
		targetExtensions,
		isExtensionSupported,
		targetExtensionCount);

	if (result == false || isExtensionSupported[swapchainExtIndex] == false)
	{
		destroyVkWindow(instance, window);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

#if __APPLE__
	if (isExtensionSupported[portabilitySubsetExtIndex] == false)
	{
		destroyVkWindow(instance, window);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}
#endif

	if (isExtensionSupported[memoryBudgetExtIndex] == true)
		extensions[extensionCount++] = targetExtensions[memoryBudgetExtIndex];

	if (useRayTracing == true)
	{
		if (isExtensionSupported[deferredHostOperationsExtIndex] == false ||
			isExtensionSupported[accelerationStructureExtIndex] == false ||
			isExtensionSupported[rayTracingPipelineExtIndex] == false)
		{
			destroyVkWindow(instance, window);
			return RAY_TRACING_IS_NOT_SUPPORTED_MPGX_RESULT;
		}
	}

	VkDevice device;

	MpgxResult mpgxResult = createVkDevice(
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

	VmaAllocator allocator = createVmaAllocator(
		physicalDevice,
		device,
		instance,
		isExtensionSupported[memoryBudgetExtIndex],
		useRayTracing);

	if (allocator == NULL)
	{
		destroyVkWindow(instance, window);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	window->allocator = allocator;

	VkQueue graphicsQueue, presentQueue;

	vkGetDeviceQueue(
		device,
		graphicsQueueFamilyIndex,
		0,
		&graphicsQueue);

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		presentQueue = graphicsQueue;
	}
	else
	{
		vkGetDeviceQueue(
			device,
			presentQueueFamilyIndex,
			0,
			&presentQueue);
	}

	window->graphicsQueue = graphicsQueue;
	window->presentQueue = presentQueue;

	VkCommandPool graphicsCommandPool = createVkCommandPool(
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		graphicsQueueFamilyIndex);

	if (graphicsCommandPool == NULL)
	{
		destroyVkWindow(instance, window);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	window->graphicsCommandPool = graphicsCommandPool;

	VkCommandPool presentCommandPool;

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		presentCommandPool = graphicsCommandPool;
	}
	else
	{
		presentCommandPool = createVkCommandPool(
			device,
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			presentQueueFamilyIndex);

		if (presentCommandPool == NULL)
		{
			destroyVkWindow(instance, window);
			return FAILED_TO_ALLOCATE_MPGX_RESULT;
		}
	}

	window->presentCommandPool = presentCommandPool;

	VkCommandPool transferCommandPool = createVkCommandPool(
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		graphicsQueueFamilyIndex);

	if (transferCommandPool == NULL)
	{
		destroyVkWindow(instance, window);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	window->transferCommandPool = transferCommandPool;

	VkFence* fences = window->fences;
	VkSemaphore* imageAcquiredSemaphores =
		window->imageAcquiredSemaphores;
	VkSemaphore* drawCompleteSemaphores =
		window->drawCompleteSemaphores;
	VkSemaphore* imageOwnershipSemaphores =
		window->imageOwnershipSemaphores;

	for (uint8_t i = 0; i < VK_FRAME_LAG; i++)
	{
		fences[i] = createVkFence(device,
			VK_FENCE_CREATE_SIGNALED_BIT);

		imageAcquiredSemaphores[i] = createVkSemaphore(device);
		drawCompleteSemaphores[i] = createVkSemaphore(device);
		imageOwnershipSemaphores[i] = createVkSemaphore(device);

		if (fences[i] == NULL ||
			imageAcquiredSemaphores[i] == NULL ||
			drawCompleteSemaphores[i] == NULL ||
			imageOwnershipSemaphores[i] == NULL)
		{
			destroyVkWindow(instance, window);
			return FAILED_TO_ALLOCATE_MPGX_RESULT;
		}
	}

	VkFence transferFence = createVkFence(
		device,
		0);

	if (transferFence == NULL)
	{
		destroyVkWindow(instance, window);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
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