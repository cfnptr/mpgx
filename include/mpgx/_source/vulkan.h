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

#include "mpgx/_source/graphics.h"
#include "mpgx/_source/swapchain.h"

#define ENGINE_NAME "MPGX"
#define VK_VERSION VK_API_VERSION_1_2
#define VK_FRAME_LAG 2

struct VkWindow
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
	VkSwapchain swapchain;
	uint32_t frameIndex;
	uint32_t bufferIndex;
	VkCommandBuffer currenCommandBuffer;
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	size_t stagingSize;
	VkFence stagingFence;
};

typedef struct VkWindow* VkWindow;

static VkBool32 VKAPI_CALL vkDebugMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
#if __APPLE__
	if (callbackData->messageIdNumber == 0x6bbb14 ||
		callbackData->messageIdNumber == 0xf467460)
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

inline static VkInstance createVkInstance(
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch,
	const char** requiredLayers,
	uint32_t requiredLayerCount,
	const char** preferredLayers,
	uint32_t preferredLayerCount,
	const char** requiredExtensions,
	uint32_t requiredExtensionCount,
	const char** preferredExtensions,
	uint32_t preferredExtensionCount,
	bool* supportedExtensions)
{
	VkResult result;
	uint32_t layerCount;
	const char** layers;

#ifndef NDEBUG
	uint32_t layerPropertyCount;

	result = vkEnumerateInstanceLayerProperties(
		&layerPropertyCount,
		NULL);

	if (result != VK_SUCCESS ||
		layerPropertyCount == 0)
	{
		return NULL;
	}

	VkLayerProperties* layerProperties = malloc(
		layerPropertyCount * sizeof(VkLayerProperties));

	if (layerProperties == NULL)
		return NULL;

	result = vkEnumerateInstanceLayerProperties(
		&layerPropertyCount,
		layerProperties);

	if (result != VK_SUCCESS ||
		layerPropertyCount == 0)
	{
		free(layerProperties);
		return NULL;
	}

	uint32_t layerSize = requiredLayerCount + preferredLayerCount;
	layers = malloc(layerSize * sizeof(const char*));

	if (layers == NULL)
	{
		free(layerProperties);
		return NULL;
	}

	layerCount = requiredLayerCount;

	for (uint32_t i = 0; i < requiredLayerCount; i++)
		layers[i] = requiredLayers[i];

	for (uint32_t i = 0; i < preferredLayerCount; i++)
	{
		bool isLayerFound = false;

		for (uint32_t j = 0; j < layerPropertyCount; j++)
		{
			if (strcmp(preferredLayers[i],
				layerProperties[j].layerName) == 0)
			{
				isLayerFound = true;
				break;
			}
		}

		if (isLayerFound == true)
		{
			isLayerFound = false;

			for (uint32_t j = 0; j < layerCount; j++)
			{
				if (strcmp(layers[j],
					preferredLayers[i]) == 0)
				{
					isLayerFound = true;
					break;
				}
			}

			if (isLayerFound == false)
				layers[layerCount++] = preferredLayers[i];
		}
	}

	free(layerProperties);
#else
	layerCount = 0;
	layers = NULL;
#endif

	uint32_t glfwExtensionCount;

	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(
		&glfwExtensionCount);

	if (glfwExtensionCount == 0 ||
		glfwExtensions == NULL)
	{
		free((void*)layers);
		return NULL;
	}

	uint32_t extensionPropertyCount;

	result = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionPropertyCount,
		NULL);

	if (result != VK_SUCCESS ||
		extensionPropertyCount == 0)
	{
		free((void*)layers);
		return NULL;
	}

	VkExtensionProperties* extensionProperties = malloc(
		extensionPropertyCount * sizeof(VkLayerProperties));

	if (extensionProperties == NULL)
	{
		free((void*)layers);
		return NULL;
	}

	result = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionPropertyCount,
		extensionProperties);

	if (result != VK_SUCCESS ||
		extensionPropertyCount == 0)
	{
		free(extensionProperties);
		free((void*)layers);
		return NULL;
	}

	uint32_t extensionSize = glfwExtensionCount +
		requiredExtensionCount + preferredExtensionCount;

	const char** extensions = malloc(
		extensionSize * sizeof(const char*));

	if (extensions == NULL)
	{
		free(extensionProperties);
		free((void*)layers);
		return NULL;
	}

	uint32_t extensionCount = glfwExtensionCount;

	for (uint32_t i = 0; i < glfwExtensionCount; i++)
		extensions[i] = glfwExtensions[i];

	for (uint32_t i = 0; i < requiredExtensionCount; i++)
	{
		bool isExtensionFound = false;

		for (uint32_t j = 0; j < extensionCount; j++)
		{
			if (strcmp(extensions[j],
				requiredExtensions[i]) == 0)
			{
				isExtensionFound = true;
				break;
			}
		}

		if (isExtensionFound == false)
			extensions[extensionCount++] = requiredExtensions[i];
	}

	for (uint32_t i = 0; i < preferredExtensionCount; i++)
	{
		bool isExtensionFound = false;

		for (uint32_t j = 0; j < extensionPropertyCount; j++)
		{
			if (strcmp(preferredExtensions[i],
				extensionProperties[j].extensionName) == 0)
			{
				isExtensionFound = true;
				break;
			}
		}

		if (isExtensionFound == true)
		{
			supportedExtensions[i] = isExtensionFound;
			isExtensionFound = false;

			for (uint32_t j = 0; j < extensionCount; j++)
			{
				if (strcmp(extensions[j],
					preferredExtensions[i]) == 0)
				{
					isExtensionFound = true;
					break;
				}
			}

			if (isExtensionFound == false)
				extensions[extensionCount++] = preferredExtensions[i];
		}
	}

	free(extensionProperties);

	uint32_t appVersion = VK_MAKE_API_VERSION(
		0,
		appVersionMajor,
		appVersionMinor,
		appVersionPatch);
	const uint32_t engineVersion = VK_MAKE_API_VERSION(
		0,
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

	result = vkCreateInstance(
		&instanceCreateInfo,
		NULL,
		&instance);

	free((void*)extensions);
	free((void*)layers);

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
			instance,
			"vkCreateDebugUtilsMessengerEXT");

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
			instance,
			"vkDestroyDebugUtilsMessengerEXT");

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

	if (result != VK_SUCCESS ||
		deviceCount == 0)
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

inline static VkDevice createVkDevice(
	VkPhysicalDevice physicalDevice,
	uint32_t graphicsQueueFamilyIndex,
	uint32_t presentQueueFamilyIndex,
	const char** requiredExtensions,
	uint32_t requiredExtensionCount,
	const char** preferredExtensions,
	uint32_t preferredExtensionCount,
	bool* supportedExtensions)
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

	uint32_t propertyCount;

	VkResult result = vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		NULL,
		&propertyCount,
		NULL);

	if (result != VK_SUCCESS ||
		propertyCount == 0)
	{
		return NULL;
	}

	VkExtensionProperties* properties = malloc(
		propertyCount * sizeof(VkExtensionProperties));

	if (properties == NULL)
		return NULL;

	result = vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		NULL,
		&propertyCount,
		properties);

	if (result != VK_SUCCESS ||
		propertyCount == 0)
	{
		free(properties);
		return NULL;
	}

	uint32_t extensionSize =
		requiredExtensionCount + preferredExtensionCount;
	const char** extensions = malloc(
		extensionSize * sizeof(const char*));

	if (extensions == NULL)
	{
		free(properties);
		return NULL;
	}

	uint32_t extensionCount = requiredExtensionCount;

	for (uint32_t i = 0; i < requiredExtensionCount; i++)
		extensions[i] = requiredExtensions[i];

	for (uint32_t i = 0; i < preferredExtensionCount; i++)
	{
		bool isExtensionFound = false;

		for (uint32_t j = 0; j < propertyCount; j++)
		{
			if (strcmp(preferredExtensions[i],
				properties[j].extensionName) == 0)
			{
				isExtensionFound = true;
				break;
			}
		}

		if (isExtensionFound == true)
		{
			supportedExtensions[i] = isExtensionFound;
			isExtensionFound = false;

			for (uint32_t j = 0; j < extensionCount; j++)
			{
				if (strcmp(extensions[j],
					preferredExtensions[i]) == 0)
				{
					isExtensionFound = true;
					break;
				}
			}

			if (isExtensionFound == false)
				extensions[extensionCount++] = preferredExtensions[i];
		}
	}

	free(properties);

	VkDeviceCreateInfo deviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		NULL,
		0,
		queueCreateInfoCount,
		queueCreateInfos,
		0,
		NULL,
		extensionCount,
		extensions,
		NULL,
	};

	VkDevice device;

	result = vkCreateDevice(
		physicalDevice,
		&deviceCreateInfo,
		NULL,
		&device);

	free(extensions);

	if (result != VK_SUCCESS)
		return NULL;

	return device;
}

inline static VmaAllocator createVmaAllocator(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkInstance instance)
{
	VmaAllocatorCreateInfo createInfo;

	memset(
		&createInfo,
		0,
		sizeof(VmaAllocatorCreateInfo));

	createInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
	createInfo.physicalDevice = physicalDevice;
	createInfo.device = device;
	createInfo.instance = instance;
	createInfo.vulkanApiVersion = VK_VERSION;

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

inline static VkWindow createVkWindow(
	Window window,
	VkInstance instance,
	GLFWwindow* handle,
	bool useStencilBuffer,
	Vec2U framebufferSize)
{
	VkWindow vkWindow = malloc(
		sizeof(struct VkWindow));

	if (vkWindow == NULL)
		return NULL;

	VkSurfaceKHR surface;

	VkResult vkResult = glfwCreateWindowSurface(
		instance,
		handle,
		NULL,
		&surface);

	if (vkResult != VK_SUCCESS)
	{
		free(vkWindow);
		return NULL;
	}

	bool isGpuIntegrated;

	VkPhysicalDevice physicalDevice = getBestVkPhysicalDevice(
		instance,
		&isGpuIntegrated);

	if (physicalDevice == NULL)
	{
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

	uint32_t graphicsQueueFamilyIndex,
		presentQueueFamilyIndex;

	bool result = getVkQueueFamilyIndices(
		physicalDevice,
		surface,
		&graphicsQueueFamilyIndex,
		&presentQueueFamilyIndex);

	if (result == false)
	{
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

	const char* requiredExtensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	const char* preferredExtensions[1];
	uint32_t preferredExtensionCount = 0;

#if __APPLE__
	preferredExtensions[preferredExtensionCount++] =
		"VK_KHR_portability_subset";
#endif

	bool supportedExtensions[1];

	VkDevice device = createVkDevice(
		physicalDevice,
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		requiredExtensions,
		1,
		preferredExtensions,
		preferredExtensionCount,
		supportedExtensions);

	if (device == NULL)
	{
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

	VmaAllocator allocator = createVmaAllocator(
		physicalDevice,
		device,
		instance);

	if (allocator == NULL)
	{
		vkDestroyDevice(
			device,
			NULL);
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

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

	VkCommandPool graphicsCommandPool = createVkCommandPool(
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		graphicsQueueFamilyIndex);

	if (graphicsCommandPool == NULL)
	{
		vmaDestroyAllocator(
			allocator);
		vkDestroyDevice(
			device,
			NULL);
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

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
			vkDestroyCommandPool(
				device,
				graphicsCommandPool,
				NULL);
			vmaDestroyAllocator(
				allocator);
			vkDestroyDevice(
				device,
				NULL);
			vkDestroySurfaceKHR(
				instance,
				surface,
				NULL);
			free(vkWindow);
			return NULL;
		}
	}

	VkCommandPool transferCommandPool = createVkCommandPool(
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		graphicsQueueFamilyIndex);

	if (transferCommandPool == NULL)
	{
		if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
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
				graphicsCommandPool,
				NULL);
			vkDestroyCommandPool(
				device,
				presentCommandPool,
				NULL);
		}

		vmaDestroyAllocator(
			allocator);
		vkDestroyDevice(
			device,
			NULL);
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

	VkSwapchain swapchain = createVkSwapchain(
		surface,
		physicalDevice,
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		device,
		allocator,
		graphicsCommandPool,
		presentCommandPool,
		useStencilBuffer,
		framebufferSize);

	if (swapchain == NULL)
	{
		vkDestroyCommandPool(
			device,
			transferCommandPool,
			NULL);

		if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
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
				graphicsCommandPool,
				NULL);
			vkDestroyCommandPool(
				device,
				presentCommandPool,
				NULL);
		}

		vmaDestroyAllocator(
			allocator);
		vkDestroyDevice(
			device,
			NULL);
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

	VkFenceCreateInfo fenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		NULL,
		0,
	};

	VkFence stagingFence = createVkFence(
		device,
		0);

	if (stagingFence == NULL)
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		vkDestroyCommandPool(
			device,
			transferCommandPool,
			NULL);

		if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
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
				graphicsCommandPool,
				NULL);
			vkDestroyCommandPool(
				device,
				presentCommandPool,
				NULL);
		}

		vmaDestroyAllocator(
			allocator);
		vkDestroyDevice(
			device,
			NULL);
		vkDestroySurfaceKHR(
			instance,
			surface,
			NULL);
		free(vkWindow);
		return NULL;
	}

	vkWindow->surface = surface;
	vkWindow->physicalDevice = physicalDevice;
	vkWindow->isGpuIntegrated = isGpuIntegrated;
	vkWindow->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	vkWindow->presentQueueFamilyIndex = presentQueueFamilyIndex;
	vkWindow->device = device;
	vkWindow->allocator = allocator;
	vkWindow->graphicsQueue = graphicsQueue;
	vkWindow->presentQueue = presentQueue;
	vkWindow->graphicsCommandPool = graphicsCommandPool;
	vkWindow->presentCommandPool = presentCommandPool;
	vkWindow->transferCommandPool = transferCommandPool;
	vkWindow->swapchain = swapchain;
	vkWindow->frameIndex = 0;
	vkWindow->bufferIndex = 0;
	vkWindow->currenCommandBuffer = NULL;
	vkWindow->stagingBuffer = NULL;
	vkWindow->stagingAllocation = NULL;
	vkWindow->stagingSize = 0;
	vkWindow->stagingFence = stagingFence;

	VkFence* fences = vkWindow->fences;

	VkSemaphore* imageAcquiredSemaphores =
		vkWindow->imageAcquiredSemaphores;
	VkSemaphore* drawCompleteSemaphores =
		vkWindow->drawCompleteSemaphores;
	VkSemaphore* imageOwnershipSemaphores =
		vkWindow->imageOwnershipSemaphores;

	for (uint8_t i = 0; i < VK_FRAME_LAG; i++)
	{
		fences[i] = createVkFence(
			device,
			VK_FENCE_CREATE_SIGNALED_BIT);

		imageAcquiredSemaphores[i] = createVkSemaphore(device);
		drawCompleteSemaphores[i] = createVkSemaphore(device);
		imageOwnershipSemaphores[i] = createVkSemaphore(device);

		if (fences[i] == NULL ||
			imageAcquiredSemaphores[i] == NULL ||
			drawCompleteSemaphores[i] == NULL ||
			imageOwnershipSemaphores[i] == NULL)
		{
			vkDestroyFence(
				device,
				fences[i],
				NULL);
			vkDestroySemaphore(
				device,
				imageAcquiredSemaphores[i],
				NULL);
			vkDestroySemaphore(
				device,
				drawCompleteSemaphores[i],
				NULL);
			vkDestroySemaphore(
				device,
				imageOwnershipSemaphores[i],
				NULL);

			vkDestroyFence(
				device,
				stagingFence,
				NULL);
			destroyVkSwapchain(
				device,
				allocator,
				graphicsCommandPool,
				presentCommandPool,
				swapchain);
			vkDestroyCommandPool(
				device,
				transferCommandPool,
				NULL);

			if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
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
					graphicsCommandPool,
					NULL);
				vkDestroyCommandPool(
					device,
					presentCommandPool,
					NULL);
			}

			vmaDestroyAllocator(
				allocator);
			vkDestroyDevice(
				device,
				NULL);
			vkDestroySurfaceKHR(
				instance,
				surface,
				NULL);
			free(vkWindow);
			return NULL;
		}
	}

	return vkWindow;
}
inline static void destroyVkWindow(
	VkInstance instance,
	VkWindow vkWindow)
{
	if (vkWindow == NULL)
		return;

	VkDevice device = vkWindow->device;

	vkDestroyFence(
		device,
		vkWindow->stagingFence,
		NULL);

	VmaAllocator allocator = vkWindow->allocator;

	if (vkWindow->stagingBuffer != NULL)
	{
		vmaDestroyBuffer(
			allocator,
			vkWindow->stagingBuffer,
			vkWindow->stagingAllocation);
	}

	VkCommandPool graphicsCommandPool =
		vkWindow->graphicsCommandPool;
	VkCommandPool presentCommandPool =
		vkWindow->presentCommandPool;

	destroyVkSwapchain(
		device,
		allocator,
		graphicsCommandPool,
		presentCommandPool,
		vkWindow->swapchain);

	VkFence* fences = vkWindow->fences;

	VkSemaphore* imageAcquiredSemaphores =
		vkWindow->imageAcquiredSemaphores;
	VkSemaphore* drawCompleteSemaphores =
		vkWindow->drawCompleteSemaphores;
	VkSemaphore* imageOwnershipSemaphores =
		vkWindow->imageOwnershipSemaphores;

	for (uint8_t i = 0; i < VK_FRAME_LAG; i++)
	{
		VkResult result = vkWaitForFences(
			device,
			1,
			&fences[i],
			VK_TRUE,
			UINT64_MAX);

		if(result != VK_SUCCESS)
			abort();

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
		vkDestroyFence(
			device,
			fences[i],
			NULL);
	}

	vkDestroyCommandPool(
		device,
		vkWindow->transferCommandPool,
		NULL);

	if (vkWindow->graphicsQueueFamilyIndex ==
		vkWindow->presentQueueFamilyIndex)
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

	vmaDestroyAllocator(
		allocator);
	vkDestroyDevice(
		device,
		NULL);
	vkDestroySurfaceKHR(
		instance,
		vkWindow->surface,
		NULL);

	free(vkWindow);
}

static VkPhysicalDeviceProperties properties;

inline static const char* getVkWindowGpuName(
	VkPhysicalDevice physicalDevice)
{
	vkGetPhysicalDeviceProperties(
		physicalDevice,
		&properties);
	return properties.deviceName;
}

inline static bool getVkCompareOperator(
	CompareOperator compareOperator,
	VkCompareOp* vkCompareOperator)
{
	switch (compareOperator)
	{
	default:
		return false;
	case LESS_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_LESS;
		return true;
	case LESS_OR_EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_LESS_OR_EQUAL;
		return true;
	case GREATER_OR_EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_GREATER_OR_EQUAL;
		return true;
	case GREATER_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_GREATER;
		return true;
	case EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_EQUAL;
		return true;
	case NOT_EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_NOT_EQUAL;
		return true;
	case ALWAYS_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_ALWAYS;
		return true;
	case NEVER_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_NEVER;
		return true;
	}
}

// TODO: implement DemoUpdateTargetIPD (VK_GOOGLE) from cube.c
// TODO: investigate if we should use (demo_flush_init_cmd) from cube.c
