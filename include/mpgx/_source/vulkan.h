#pragma once
#if MPGX_SUPPORT_VULKAN

#include <string.h>

static VkBool32 VKAPI_CALL vkDebugMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
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
		pCallbackData->pMessage);
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
	uint32_t preferredExtensionCount)
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

	if (glfwExtensionCount == 0)
	{
		free(layers);
		return NULL;
	}

	uint32_t extensionProperyCount;

	result = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionProperyCount,
		NULL);

	if (result != VK_SUCCESS ||
		extensionProperyCount == 0)
	{
		free(layers);
		return NULL;
	}

	VkExtensionProperties* extensionProperties = malloc(
		extensionProperyCount * sizeof(VkLayerProperties));

	if (extensionProperties == NULL)
	{
		free(layers);
		return NULL;
	}

	result = vkEnumerateInstanceExtensionProperties(
		NULL,
		&extensionProperyCount,
		extensionProperties);

	if (result != VK_SUCCESS ||
		extensionProperyCount == 0)
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

		for (uint32_t j = 0; j < extensionProperyCount; j++)
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
		"MPGX",
		engineVersion,
		VK_API_VERSION_1_0,
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
			VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
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

			instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
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
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
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

	PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessenger =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");

	if (createDebugUtilsMessenger == NULL)
		return NULL;

	VkDebugUtilsMessengerEXT debugUtilsMessenger;

	VkResult result = createDebugUtilsMessenger(
		instance,
		&debugUtilsMessengerCreateInfo,
		NULL,
		&debugUtilsMessenger);

	if (result != VK_SUCCESS)
		return NULL;

	return debugUtilsMessenger;
}
inline static void destroyVkDebugUtilsMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugUtilsMessenger)
{
	if (debugUtilsMessenger == NULL)
		return;

	PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessenger =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");

	if (destroyDebugUtilsMessenger == NULL)
		abort();

	destroyDebugUtilsMessenger(
		instance,
		debugUtilsMessenger,
		NULL);
}

inline static VkSurfaceKHR createVkSurface(
	VkInstance vkInstance,
	GLFWwindow* glfwWindow)
{
	VkSurfaceKHR vkSurface;

	VkResult result = glfwCreateWindowSurface(
		vkInstance,
		glfwWindow,
		NULL,
		&vkSurface);

	if (result != VK_SUCCESS)
		return NULL;

	return vkSurface;
}
inline static void destroyVkSurface(
	VkInstance vkInstance,
	VkSurfaceKHR vkSurface)
{
	vkDestroySurfaceKHR(
		vkInstance,
		vkSurface,
		NULL);
}

inline static VkPhysicalDevice getBestVkPhysicalDevice(
	VkInstance vkInstance)
{
	uint32_t physicalDeviceCount;

	VkResult result = vkEnumeratePhysicalDevices(
		vkInstance,
		&physicalDeviceCount,
		NULL);

	if (result != VK_SUCCESS)
		return NULL;

	VkPhysicalDevice* physicalDevices = malloc(
		physicalDeviceCount * sizeof(VkPhysicalDevice));

	if (physicalDevices == NULL)
		return NULL;

	result = vkEnumeratePhysicalDevices(
		vkInstance,
		&physicalDeviceCount,
		physicalDevices);

	if (result != VK_SUCCESS)
	{
		free(physicalDevices);
		return NULL;
	}

	VkPhysicalDevice targetPhysicalDevice = NULL;
	uint32_t targetScore = 0;

	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		VkPhysicalDevice physicalDevice = physicalDevices[i];
		VkPhysicalDeviceProperties physicalDeviceProperties;

		vkGetPhysicalDeviceProperties(
			physicalDevice,
			&physicalDeviceProperties);

		uint32_t score = 0;

		if (physicalDeviceProperties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}
		else if (physicalDeviceProperties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		{
			score += 750;
		}
		else if (physicalDeviceProperties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			score += 500;
		}
		else if (physicalDeviceProperties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_CPU)
		{
			score += 250;
		}

		// TODO: add other tests

		if (score > targetScore)
		{
			targetPhysicalDevice = physicalDevice;
			targetScore = score;
		}
	}

	free(physicalDevices);
	return targetPhysicalDevice;
}
#endif
