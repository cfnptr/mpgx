#pragma once
#include "mpgx/_source/swapchain.h"
#include <string.h>

#define ENGINE_NAME "MPGX"
#define VK_VERSION VK_API_VERSION_1_2
#define VK_FRAME_LAG 2

struct VkWindow
{
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;
	VkDevice device;
	VmaAllocator vmaAllocator;
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
};

typedef struct VkWindow* VkWindow;

static VkBool32 VKAPI_CALL vkDebugMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
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
		free(layers);
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
		free(layers);
		return NULL;
	}

	VkExtensionProperties* extensionProperties = malloc(
		extensionPropertyCount * sizeof(VkLayerProperties));

	if (extensionProperties == NULL)
	{
		free(layers);
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
		free(layers);
		return NULL;
	}

	uint32_t extensionSize = glfwExtensionCount +
		requiredExtensionCount + preferredExtensionCount;

	const char** extensions = malloc(
		extensionSize * sizeof(const char*));

	if (extensions == NULL)
	{
		free(extensionProperties);
		free(layers);
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

		supportedExtensions[i] = isExtensionFound;
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

	free(extensions);
	free(layers);

	if (result != VK_SUCCESS)
		return NULL;

	return instance;
}
inline static void destroyVkInstance(VkInstance instance)
{
	if (instance == NULL)
		return;

	vkDestroyInstance(
		instance,
		NULL);
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

inline static VkSurfaceKHR createVkSurface(
	VkInstance instance,
	GLFWwindow* glfwWindow)
{
	VkSurfaceKHR surface;

	VkResult result = glfwCreateWindowSurface(
		instance,
		glfwWindow,
		NULL,
		&surface);

	if (result != VK_SUCCESS)
		return NULL;

	return surface;
}
inline static void destroyVkSurface(
	VkInstance instance,
	VkSurfaceKHR surface)
{
	vkDestroySurfaceKHR(
		instance,
		surface,
		NULL);
}

inline static VkPhysicalDevice getBestVkPhysicalDevice(
	VkInstance instance)
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
		}
	}

	free(devices);
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
		return NULL;

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

		supportedExtensions[i] = isExtensionFound;
	}

	free(properties);

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

	// TODO: enabled device features

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
inline static void destroyVkDevice(
	VkDevice device)
{
	vkDestroyDevice(
		device,
		NULL);
}

inline static VmaAllocator createVmaAllocator(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkInstance instance)
{
	VmaAllocatorCreateInfo createInfo = {};
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
inline static void destroyVmaAllocator(
	VmaAllocator allocator)
{
	vmaDestroyAllocator(allocator);
}

inline static VkQueue getVkQueue(
	VkDevice device,
	uint32_t queueFamilyIndex)
{
	VkQueue queue;

	vkGetDeviceQueue(
		device,
		queueFamilyIndex,
		0,
		&queue);

	return queue;
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
inline static void destroyVkCommandPool(
	VkDevice device,
	VkCommandPool commandPool)
{
	vkDestroyCommandPool(
		device,
		commandPool,
		NULL);
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
inline static void destroyVkFence(
	VkDevice device,
	VkFence fence)
{
	vkDestroyFence(
		device,
		fence,
		NULL);
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
inline static void destroyVkSemaphore(
	VkDevice device,
	VkSemaphore semaphore)
{
	vkDestroySemaphore(
		device,
		semaphore,
		NULL);
}

inline static VkWindow createVkWindow(
	VkInstance instance,
	GLFWwindow* handle,
	bool useStencilBuffer,
	Vec2U framebufferSize)
{
	VkWindow window = malloc(
		sizeof(struct VkWindow));

	if (window == NULL)
		return NULL;

	VkSurfaceKHR surface = createVkSurface(
		instance,
		handle);

	if (surface == NULL)
	{
		free(window);
		return NULL;
	}

	VkPhysicalDevice physicalDevice =
		getBestVkPhysicalDevice(instance);

	if (physicalDevice == NULL)
	{
		destroyVkSurface(instance, surface);
		free(window);
		return NULL;
	}

	uint32_t
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex;

	bool result = getVkQueueFamilyIndices(
		physicalDevice,
		surface,
		&graphicsQueueFamilyIndex,
		&presentQueueFamilyIndex);

	if (result == false)
	{
		destroyVkSurface(instance, surface);
		free(window);
		return NULL;
	}

	const char* requiredExtensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	const char* preferredExtensions[3] = {
		"VK_KHR_portability_subset",
	};

	bool supportedExtensions[1];

	VkDevice device = createVkDevice(
		physicalDevice,
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		requiredExtensions,
		1,
		preferredExtensions,
		1,
		supportedExtensions);

	if (device == NULL)
	{
		destroyVkSurface(instance, surface);
		free(window);
		return NULL;
	}

	VmaAllocator vmaAllocator = createVmaAllocator(
		physicalDevice,
		device,
		instance);

	if (vmaAllocator == NULL)
	{
		destroyVkDevice(device);
		destroyVkSurface(instance, surface);
		free(window);
		return NULL;
	}

	VkQueue graphicsQueue, presentQueue;
	VkCommandPool graphicsCommandPool, presentCommandPool;

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		graphicsQueue = presentQueue = getVkQueue(
			device,
			graphicsQueueFamilyIndex);
	}
	else
	{
		graphicsQueue = getVkQueue(
			device,
			graphicsQueueFamilyIndex);
		presentQueue = getVkQueue(
			device,
			graphicsQueueFamilyIndex);
	}

	if (graphicsQueue == NULL || presentQueue == NULL)
	{
		destroyVmaAllocator(vmaAllocator);
		destroyVkDevice(device);
		destroyVkSurface(instance, surface);
		free(window);
		return NULL;
	}

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		graphicsCommandPool = presentCommandPool = createVkCommandPool(
			device,
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			graphicsQueueFamilyIndex);
	}
	else
	{
		graphicsCommandPool = createVkCommandPool(
			device,
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			graphicsQueueFamilyIndex);
		presentCommandPool = createVkCommandPool(
			device,
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			presentQueueFamilyIndex);
	}

	if (graphicsCommandPool == NULL || presentCommandPool == NULL)
	{
		destroyVkCommandPool(device, graphicsCommandPool);
		destroyVkCommandPool(device, presentCommandPool);
		destroyVmaAllocator(vmaAllocator);
		destroyVkDevice(device);
		destroyVkSurface(instance, surface);
		free(window);
		return NULL;
	}

	// TODO: investigate why we require separated transfer pool
	VkCommandPool transferCommandPool = createVkCommandPool(
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		graphicsQueueFamilyIndex);

	if (transferCommandPool == NULL)
	{
		if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
		{
			destroyVkCommandPool(device, graphicsCommandPool);
		}
		else
		{
			destroyVkCommandPool(device, graphicsCommandPool);
			destroyVkCommandPool(device, presentCommandPool);
		}

		destroyVmaAllocator(vmaAllocator);
		destroyVkDevice(device);
		destroyVkSurface(instance, surface);
		free(window);
		return NULL;
	}

	VkSwapchain swapchain = createVkSwapchain(
		physicalDevice,
		surface,
		device,
		vmaAllocator,
		graphicsCommandPool,
		presentCommandPool,
		useStencilBuffer,
		framebufferSize,
		NULL);

	window->surface = surface;
	window->physicalDevice = physicalDevice;
	window->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	window->presentQueueFamilyIndex = presentQueueFamilyIndex;
	window->device = device;
	window->vmaAllocator = vmaAllocator;
	window->graphicsQueue = graphicsQueue;
	window->presentQueue = presentQueue;
	window->graphicsCommandPool = graphicsCommandPool;
	window->presentCommandPool = presentCommandPool;
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
			destroyVkFence(device, fences[i]);
			destroyVkSemaphore(device, imageAcquiredSemaphores[i]);
			destroyVkSemaphore(device, drawCompleteSemaphores[i]);
			destroyVkSemaphore(device, imageOwnershipSemaphores[i]);

			if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
			{
				destroyVkCommandPool(device, graphicsCommandPool);
			}
			else
			{
				destroyVkCommandPool(device, graphicsCommandPool);
				destroyVkCommandPool(device, presentCommandPool);
			}

			destroyVmaAllocator(vmaAllocator);
			destroyVkDevice(device);
			destroyVkSurface(instance, surface);
			free(window);
			return NULL;
		}
	}

	return window;
}
inline static void destroyVkWindow(
	VkInstance instance,
	VkWindow window)
{
	if (window == NULL)
		return;

	VkDevice device = window->device;
	VkFence* fences = window->fences;

	VkSemaphore* imageAcquiredSemaphores =
		window->imageAcquiredSemaphores;
	VkSemaphore* drawCompleteSemaphores =
		window->drawCompleteSemaphores;
	VkSemaphore* imageOwnershipSemaphores =
		window->imageOwnershipSemaphores;

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

		destroyVkFence(device, fences[i]);
		destroyVkSemaphore(device, imageAcquiredSemaphores[i]);
		destroyVkSemaphore(device, drawCompleteSemaphores[i]);
		destroyVkSemaphore(device, imageOwnershipSemaphores[i]);
	}

	destroyVkCommandPool(
		device,
		window->transferCommandPool);

	if (window->graphicsQueueFamilyIndex ==
		window->presentQueueFamilyIndex)
	{
		destroyVkCommandPool(
			device,
			window->graphicsCommandPool);
	}
	else
	{
		destroyVkCommandPool(
			device,
			window->graphicsCommandPool);
		destroyVkCommandPool(
			device,
			window->presentCommandPool);
	}

	destroyVmaAllocator(window->vmaAllocator);
	destroyVkDevice(device);

	destroyVkSurface(
		instance,
		window->surface);

	free(window);
}
inline static void waitVkWindow(VkWindow window)
{
	VkResult result = vkDeviceWaitIdle(
		window->device);

	if (result != VK_SUCCESS)
		abort();
}
