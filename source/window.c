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

#include "mpgx/window.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_mesh.h"
#include "mpgx/_source/sampler.h"
#include "mpgx/_source/framebuffer.h"
#include "mpgx/_source/compute_pipeline.h"
#include "mpgx/_source/ray_tracing_pipeline.h"

#include "cmmt/common.h"
#include "mpmt/common.h"

#include <stdio.h>

// TODO: add VMA defragmentation

struct Window_T
{
	Window parent;
	bool useVsync;
	bool useStencilBuffer;
	bool useBeginClear;
	bool useRayTracing;
	uint8_t _alignment[4];
	OnWindowUpdate onUpdate;
	void* updateArgument;
	GLFWwindow* handle;
	CursorType cursorType;
	GLFWcursor* ibeamCursor;
	GLFWcursor* crosshairCursor;
	GLFWcursor* handCursor;
	GLFWcursor* hresizeCursor;
	GLFWcursor* vresizeCursor;
	uint32_t* inputBuffer;
	size_t inputCapacity;
	size_t inputLength;
#if MPGX_SUPPORT_VULKAN
	VkWindow vkWindow;
#endif
	RayTracing rayTracing;
	Framebuffer framebuffer;
	Buffer* buffers;
	size_t bufferCapacity;
	size_t bufferCount;
	Image* images;
	size_t imageCapacity;
	size_t imageCount;
	Sampler* samplers;
	size_t samplerCapacity;
	size_t samplerCount;
	Framebuffer* framebuffers;
	size_t framebufferCapacity;
	size_t framebufferCount;
	Shader* shaders;
	size_t shaderCapacity;
	size_t shaderCount;
	GraphicsMesh* graphicsMeshes;
	size_t graphicsMeshCapacity;
	size_t graphicsMeshCount;
	ComputePipeline* computePipelines;
	size_t computePipelineCapacity;
	size_t computePipelineCount;
	double updateTime;
	double deltaTime;
	Framebuffer renderFramebuffer;
	bool isRecording;
};

static bool graphicsInitialized = false;
static GraphicsAPI graphicsAPI = VULKAN_GRAPHICS_API;
static Window currentWindow = NULL;

#if MPGX_SUPPORT_VULKAN
static VkInstance vkInstance = NULL;
#ifndef NDEBUG
static VkDebugUtilsMessengerEXT vkDebugUtilsMessenger = NULL;
#endif
#endif

static void glfwErrorCallback(
	int code, const char* description)
{
	fprintf(stderr,
		"GLFW ERROR [%d]: %s\n",
		code, description);
}

MpgxResult initializeGraphics(
	GraphicsAPI api,
	const char* engineName,
	uint8_t engineVersionMajor,
	uint8_t engineVersionMinor,
	uint8_t engineVersionPatch,
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch)
{
	assert(engineName);
	assert(appName);

	if (graphicsInitialized)
		return ALREADY_INITIALIZED_MPGX_RESULT;

	if(!glfwInit())
		return FAILED_TO_INITIALIZE_MPGX_RESULT;

	glfwSetErrorCallback(glfwErrorCallback);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		uint32_t glfwExtensionCount;

		const char** glfwExtensions =
			glfwGetRequiredInstanceExtensions(
			&glfwExtensionCount);

		if (!glfwExtensions)
		{
			glfwTerminate();
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		}

		const char* layers[1];
		const char* targetLayers[1];
		bool isLayerSupported[1];
		uint32_t layerCount = 0;
		uint32_t targetLayerCount = 0;

#ifndef NDEBUG
		targetLayers[targetLayerCount] =
			"VK_LAYER_KHRONOS_validation";
		uint32_t validationLayerIndex = targetLayerCount++;
#endif

		MpgxResult mpgxResult = checkVkInstanceLayers(
			targetLayers,
			isLayerSupported,
			targetLayerCount);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			glfwTerminate();
			return mpgxResult;
		}

#ifndef NDEBUG
		if (isLayerSupported[validationLayerIndex])
			layers[layerCount++] = targetLayers[validationLayerIndex];
#endif

		const char* targetExtensions[1];
		bool isExtensionSupported[1];
		uint32_t extensionCount = glfwExtensionCount;
		uint32_t targetExtensionCount = 0;

		const char** extensions = malloc(
			(1 + glfwExtensionCount) * sizeof(const char*));

		if (!extensions)
		{
			glfwTerminate();
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		for (uint32_t i = 0; i < glfwExtensionCount; i++)
			extensions[i] = glfwExtensions[i];

#ifndef NDEBUG
		extensions[extensionCount++] =
		targetExtensions[targetExtensionCount] =
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		uint32_t debugUtilsExtIndex = targetExtensionCount++;
#endif

		mpgxResult = checkVkInstanceExtensions(
			targetExtensions,
			isExtensionSupported,
			targetExtensionCount);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			free((void*)extensions);
			glfwTerminate();
			return mpgxResult;
		}

#ifndef NDEBUG
		if (!isExtensionSupported[debugUtilsExtIndex])
		{
			free((void*)extensions);
			glfwTerminate();
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		}
#endif

		mpgxResult = createVkInstance(
			engineName,
			engineVersionMajor,
			engineVersionMinor,
			engineVersionPatch,
			appName,
			appVersionMajor,
			appVersionMinor,
			appVersionPatch,
			layers,
			layerCount,
			extensions,
			extensionCount,
			&vkInstance);

		free((void*)extensions);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			glfwTerminate();
			return mpgxResult;
		}

#ifndef NDEBUG
		mpgxResult = createVkDebugUtilsMessenger(
			vkInstance,
			&vkDebugUtilsMessenger);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			vkDestroyInstance(
				vkInstance,
				NULL);
			glfwTerminate();
			return mpgxResult;
		}
#endif

#else
		glfwTerminate();
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
#endif
	}

	graphicsInitialized = true;
	graphicsAPI = api;
	return SUCCESS_MPGX_RESULT;
}
void terminateGraphics()
{
	if (!graphicsInitialized)
		return;

#if MPGX_SUPPORT_VULKAN
	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#ifndef NDEBUG
		destroyVkDebugUtilsMessenger(
			vkInstance,
			vkDebugUtilsMessenger);
#endif
		vkDestroyInstance(
			vkInstance,
			NULL);
	}
#endif

	glfwTerminate();

	graphicsInitialized = false;
	graphicsAPI = VULKAN_GRAPHICS_API;
	currentWindow = NULL;

#if MPGX_SUPPORT_VULKAN
	vkInstance = NULL;
#ifndef NDEBUG
	vkDebugUtilsMessenger = NULL;
#endif
#endif
}
bool isGraphicsInitialized()
{
	return graphicsInitialized;
}
GraphicsAPI getGraphicsAPI()
{
	assert(graphicsInitialized);
	return graphicsAPI;
}

static void onWindowChar(
	GLFWwindow* handle,
	unsigned int codepoint)
{
	assert(handle);

	Window window = glfwGetWindowUserPointer(handle);

	size_t length = window->inputLength;

	if (length == window->inputCapacity)
	{
		size_t capacity = window->inputCapacity * 2;

		uint32_t* inputBuffer = realloc(
			window->inputBuffer,
			capacity * sizeof(uint32_t));

		if (!inputBuffer)
			abort();

		window->inputBuffer = inputBuffer;
		window->inputCapacity = capacity;
	}

	window->inputBuffer[length] = codepoint;
	window->inputLength = length + 1;
}

inline static void assertGLFW()
{
#ifndef NDEBUG
	const char* description;
	int code = glfwGetError(&description);

	if (code != GLFW_NO_ERROR)
	{
		fprintf(stderr,
			"GLFW ERROR [%d]: %s\n",
			code, description);
	}
#endif
}

#if MPGX_SUPPORT_VULKAN
inline static MpgxResult onVkResize(
	Window window,
	Vec2I newSize)
{
	assert(window);
	assert(newSize.x > 0);
	assert(newSize.y > 0);

	VkWindow vkWindow = window->vkWindow;
	VkDevice device = vkWindow->device;
	VkSwapchain swapchain = vkWindow->swapchain;

	MpgxResult mpgxResult = resizeVkSwapchain(
		vkWindow->surface,
		vkWindow->physicalDevice,
		vkWindow->graphicsQueueFamilyIndex,
		vkWindow->presentQueueFamilyIndex,
		device,
		vkWindow->allocator,
		vkWindow->graphicsCommandPool,
		vkWindow->presentCommandPool,
		vkWindow->swapchain,
		window->useVsync,
		window->useStencilBuffer,
		window->framebuffer->base.useBeginClear,
		newSize);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	Framebuffer framebuffer = window->framebuffer;

	VkSwapchainBuffer firstBuffer = swapchain->buffers[0];
	framebuffer->vk.size = newSize;
	framebuffer->vk.renderPass = swapchain->renderPass;
	framebuffer->vk.handle = firstBuffer.framebuffer;

	GraphicsPipeline* graphicsPipelines = framebuffer->vk.graphicsPipelines;
	size_t graphicsPipelineCount = framebuffer->vk.graphicsPipelineCount;

	for (size_t i = 0; i < graphicsPipelineCount; i++)
	{
		GraphicsPipeline graphicsPipeline = graphicsPipelines[i];
		VkGraphicsPipelineCreateData createData;

		mpgxResult = graphicsPipeline->vk.onResize(
			graphicsPipeline,
			newSize,
			&createData);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		mpgxResult = recreateVkGraphicsPipelineHandle(
			device,
			framebuffer->vk.renderPass,
			graphicsPipeline,
			framebuffer->vk.colorAttachmentCount,
			&createData);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;
	}

	vkWindow->frameIndex = 0;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createWindow(
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool useStencilBuffer,
	bool useBeginClear,
	bool useRayTracing,
	Window parent,
	Window* window)
{
	assert(onUpdate);
	assert(updateArgument);
	assert(window);

	if (!graphicsInitialized)
		return NOT_INITIALIZED_MPGX_RESULT;

	glfwDefaultWindowHints();

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_NO_API);
#else
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		if (useRayTracing)
			return RAY_TRACING_IS_NOT_SUPPORTED_MPGX_RESULT;

		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_OPENGL_API);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MAJOR,
			3);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MINOR,
			3);
		glfwWindowHint(
			GLFW_OPENGL_FORWARD_COMPAT,
			GLFW_TRUE);
		glfwWindowHint(
			GLFW_OPENGL_PROFILE,
			GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(
			GLFW_SRGB_CAPABLE,
			GLFW_TRUE);

		if (useStencilBuffer)
		{
			glfwWindowHint(
				GLFW_DEPTH_BITS,
				24);
			glfwWindowHint(
				GLFW_STENCIL_BITS,
				8);
		}
		else
		{
			glfwWindowHint(
				GLFW_DEPTH_BITS,
				24);
			glfwWindowHint(
				GLFW_STENCIL_BITS,
				0);
		}

#ifndef NDEBUG
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_TRUE);
#else
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_FALSE);
#endif
#else
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
#endif
	}
	else if (graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		if (useRayTracing)
			return RAY_TRACING_IS_NOT_SUPPORTED_MPGX_RESULT;

		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_OPENGL_ES_API);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MAJOR,
			3);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MINOR,
			0);
		glfwWindowHint(
			GLFW_SRGB_CAPABLE,
			GLFW_TRUE);

		if (useStencilBuffer)
		{
			glfwWindowHint(
				GLFW_DEPTH_BITS,
				24);
			glfwWindowHint(
				GLFW_STENCIL_BITS,
				8);
		}
		else
		{
			glfwWindowHint(
				GLFW_DEPTH_BITS,
				32);
			glfwWindowHint(
				GLFW_STENCIL_BITS,
				0);
		}

#ifndef NDEBUG
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_TRUE);
#else
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_FALSE);
#endif
#else
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
#endif
	}
	else
	{
		abort();
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	Window windowInstance = calloc(1,
		sizeof(Window_T));

	if (!windowInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	windowInstance->parent = parent;
	windowInstance->useVsync = true;
	windowInstance->useStencilBuffer = useStencilBuffer;
	windowInstance->useBeginClear = useBeginClear;
	windowInstance->useRayTracing = useRayTracing;
	windowInstance->onUpdate = onUpdate;
	windowInstance->updateArgument = updateArgument;

	GLFWwindow* shareWindow = parent ? parent->handle : NULL;

	GLFWwindow* handle = glfwCreateWindow(
		(int)DEFAULT_WINDOW_WIDTH,
		(int)DEFAULT_WINDOW_HEIGHT,
		DEFAULT_WINDOW_TITLE,
		NULL, // TODO: add monitor support
		shareWindow);

	if (!handle)
	{
		assertGLFW();
		destroyWindow(windowInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	windowInstance->handle = handle;

	glfwSetWindowSizeLimits(
		handle,
		2,
		2,
		GLFW_DONT_CARE,
		GLFW_DONT_CARE);

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(
			handle,
			GLFW_RAW_MOUSE_MOTION,
			GLFW_TRUE);
	}

	glfwSetWindowUserPointer(
		handle,
		windowInstance);
	glfwSetCharCallback(
		handle,
		onWindowChar);

	GLFWcursor* ibeamCursor = glfwCreateStandardCursor(
		GLFW_IBEAM_CURSOR);

	if (!ibeamCursor)
	{
		assertGLFW();
		destroyWindow(windowInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	windowInstance->cursorType = DEFAULT_CURSOR_TYPE;
	windowInstance->ibeamCursor = ibeamCursor;

	GLFWcursor* crosshairCursor = glfwCreateStandardCursor(
		GLFW_CROSSHAIR_CURSOR);

	if (!crosshairCursor)
	{
		assertGLFW();
		destroyWindow(windowInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	windowInstance->crosshairCursor = crosshairCursor;

	GLFWcursor* handCursor = glfwCreateStandardCursor(
		GLFW_HAND_CURSOR);

	if (!handCursor)
	{
		assertGLFW();
		destroyWindow(windowInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	windowInstance->handCursor = handCursor;

	GLFWcursor* hresizeCursor = glfwCreateStandardCursor(
		GLFW_HRESIZE_CURSOR);

	if (!hresizeCursor)
	{
		assertGLFW();
		destroyWindow(windowInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	windowInstance->hresizeCursor = hresizeCursor;

	GLFWcursor* vresizeCursor = glfwCreateStandardCursor(
		GLFW_VRESIZE_CURSOR);

	if (!vresizeCursor)
	{
		assertGLFW();
		destroyWindow(windowInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	windowInstance->vresizeCursor = vresizeCursor;

	uint32_t* inputBuffer = malloc(sizeof(uint32_t));

	if (!inputBuffer)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->inputBuffer = inputBuffer;
	windowInstance->inputCapacity = 1;
	windowInstance->inputLength = 0;

#if MPGX_SUPPORT_VULKAN
	VkWindow vkWindow = NULL;
#endif

	int width, height;

	glfwGetFramebufferSize(
		handle,
		&width,
		&height);

	Vec2I framebufferSize = vec2I(width, height);

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		MpgxResult mpgxResult = createVkWindow(
			vkInstance,
			handle,
			true,
			useStencilBuffer,
			useBeginClear,
			useRayTracing,
			framebufferSize,
			&vkWindow);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyWindow(windowInstance);
			return mpgxResult;
		}

		windowInstance->vkWindow = vkWindow;

		if (useRayTracing)
		{
			RayTracing rayTracing;

			mpgxResult = createVkRayTracing(
				vkInstance,
				vkWindow->physicalDevice,
				&rayTracing);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				destroyWindow(windowInstance);
				return mpgxResult;
			}

			windowInstance->rayTracing = rayTracing;
		}
		else
		{
			windowInstance->rayTracing = NULL;
		}

		VkSwapchain swapchain = vkWindow->swapchain;
		VkSwapchainBuffer firstBuffer = swapchain->buffers[0];

		Framebuffer framebuffer;

		mpgxResult = createVkDefaultFramebuffer(
			vkWindow->device,
			swapchain->renderPass,
			firstBuffer.framebuffer,
			windowInstance,
			framebufferSize,
			useBeginClear,
			&framebuffer);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyWindow(windowInstance);
			return mpgxResult;
		}

		windowInstance->framebuffer = framebuffer;
#else
		abort();
#endif
	}
	else
	{
#if MPGX_SUPPORT_OPENGL
		glfwMakeContextCurrent(handle);

		if (gladLoadGL() == 0)
		{
			destroyWindow(windowInstance);
			return FAILED_TO_INITIALIZE_MPGX_RESULT;
		}

		glfwSwapInterval(1);
		glEnable(GL_FRAMEBUFFER_SRGB);

		Framebuffer framebuffer;

		MpgxResult mpgxResult = createGlDefaultFramebuffer(
			windowInstance,
			framebufferSize,
			useBeginClear,
			&framebuffer);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyWindow(windowInstance);
			return mpgxResult;
		}

		windowInstance->framebuffer = framebuffer;
#else
		abort();
#endif
	}

	Buffer* buffers = malloc(sizeof(Buffer));

	if (!buffers)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->buffers = buffers;
	windowInstance->bufferCapacity = 1;
	windowInstance->bufferCount = 0;

	Image* images = malloc(sizeof(Image));

	if (!images)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->images = images;
	windowInstance->imageCapacity = 1;
	windowInstance->imageCount = 0;

	Sampler* samplers = malloc(sizeof(Sampler));

	if (!samplers)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->samplers = samplers;
	windowInstance->samplerCapacity = 1;
	windowInstance->samplerCount = 0;

	Shader* shaders = malloc(sizeof(Shader));

	if (!shaders)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->shaders = shaders;
	windowInstance->shaderCapacity = 1;
	windowInstance->shaderCount = 0;

	Framebuffer* framebuffers = malloc(sizeof(Framebuffer));

	if (!framebuffers)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->framebuffers = framebuffers;
	windowInstance->framebufferCapacity = 1;
	windowInstance->framebufferCount = 0;

	GraphicsMesh* graphicsMeshes = malloc(sizeof(GraphicsMesh));

	if (!graphicsMeshes)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->graphicsMeshes = graphicsMeshes;
	windowInstance->graphicsMeshCapacity = 1;
	windowInstance->graphicsMeshCount = 0;

	ComputePipeline* computePipelines = malloc(sizeof(ComputePipeline));

	if (!computePipelines)
	{
		destroyWindow(windowInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	windowInstance->computePipelines = computePipelines;
	windowInstance->computePipelineCapacity = 1;
	windowInstance->computePipelineCount = 0;

	windowInstance->updateTime = 0.0;
	windowInstance->deltaTime = 0.0;
	windowInstance->renderFramebuffer = NULL;
#ifndef NDEBUG
	windowInstance->isRecording = false;
#endif

	currentWindow = windowInstance;
	*window = windowInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyWindow(Window window)
{
	if (!window)
		return;

	assert(window->bufferCount == 0);
	assert(window->imageCount == 0);
	assert(window->samplerCount == 0);
	assert(window->framebufferCount == 0);
	assert(window->shaderCount == 0);
	assert(window->graphicsMeshCount == 0);
	assert(window->computePipelineCount == 0);
	assert(graphicsInitialized);

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		if (vkWindow)
		{
			VkDevice device = vkWindow->device;
			VkResult result = vkDeviceWaitIdle(device);

			if (result != VK_SUCCESS)
				abort();

			destroyVkFramebuffer(
				device,
				window->framebuffer);
			destroyVkRayTracing(
				window->rayTracing);
			destroyVkWindow(
				vkInstance,
				vkWindow);
		}
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		destroyGlFramebuffer(window->framebuffer);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	free(window->computePipelines);
	free(window->graphicsMeshes);
	free(window->shaders);
	free(window->framebuffers);
	free(window->samplers);
	free(window->images);
	free(window->buffers);
	free(window->inputBuffer);

	glfwDestroyCursor(window->vresizeCursor);
	glfwDestroyCursor(window->hresizeCursor);
	glfwDestroyCursor(window->handCursor);
	glfwDestroyCursor(window->crosshairCursor);
	glfwDestroyCursor(window->ibeamCursor);
	glfwDestroyWindow(window->handle);

	free(window);
}

Window getWindowParent(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->parent;
}
bool isWindowUseStencilBuffer(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->useStencilBuffer;
}
bool isWindowUseRayTracing(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->useRayTracing;
}
OnWindowUpdate getWindowOnUpdate(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->onUpdate;
}
void* getWindowUpdateArgument(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->updateArgument;
}
const uint32_t* getWindowInputBuffer(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->inputBuffer;
}
size_t getWindowInputLength(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->inputLength;
}
Framebuffer getWindowFramebuffer(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->framebuffer;
}
double getWindowUpdateTime(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->updateTime;
}
double getWindowDeltaTime(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->deltaTime;
}
Vec2F getWindowContentScale(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	float x, y;

	glfwGetWindowContentScale(
		window->handle,
		&x,&y);

	return vec2F(x, y);
}

static char gpuDriver[32];

const char* getWindowGpuName(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return window->vkWindow->deviceProperties.deviceName;
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return (const char*)glGetString(GL_RENDERER);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}
const char* getWindowGpuDriver(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		uint32_t apiVersion = window->vkWindow->deviceProperties.apiVersion;
		uint32_t driverVersion = window->vkWindow->deviceProperties.driverVersion;

#if __APPLE__
		const char* fmt = "MoltenVK %hhu.%hhu.%hhu %hhu.%hhu.%hhu";
#else
		const char* fmt = "%hhu.%hhu.%hhu %hhu.%hhu.%hhu";
#endif

		sprintf(gpuDriver, fmt,
			(uint8_t)VK_API_VERSION_MAJOR(apiVersion),
			(uint8_t)VK_API_VERSION_MINOR(apiVersion),
			(uint8_t)VK_API_VERSION_PATCH(apiVersion),
			(uint8_t)VK_API_VERSION_MAJOR(driverVersion),
			(uint8_t)VK_API_VERSION_MINOR(driverVersion),
			(uint8_t)VK_API_VERSION_PATCH(driverVersion));

		return gpuDriver;
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return (const char*)glGetString(GL_VERSION);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

void* getVkWindow(Window window)
{
	assert(window);
	assert(graphicsAPI == VULKAN_GRAPHICS_API);
	assert(graphicsInitialized);

#if MPGX_SUPPORT_VULKAN
	return window->vkWindow;
#else
	abort();
#endif
}
bool isVkDeviceIntegrated(Window window)
{
	assert(window);
	assert(graphicsAPI == VULKAN_GRAPHICS_API);
	assert(graphicsInitialized);

#if MPGX_SUPPORT_VULKAN
	return window->vkWindow->isDeviceIntegrated;
#else
	abort();
#endif
}

bool getWindowKeyboardKey(
	Window window,
	KeyboardKey key)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetKey(
		window->handle,
		key) == GLFW_PRESS;
}
bool getWindowMouseButton(
	Window window,
	MouseButton button)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetMouseButton(
		window->handle,
		button) == GLFW_PRESS;
}

bool isWindowUseVsync(
	Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->useVsync;
}
void setWindowUseVsync(
	Window window,
	bool useVsync)
{
	assert(window);
	assert(graphicsInitialized);

	if (useVsync == window->useVsync)
		return;

	window->useVsync = useVsync;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		Vec2I newSize = getFramebufferSize(window->framebuffer);
		MpgxResult mpgxResult = onVkResize(window, newSize);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			abort();
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		glfwSwapInterval(useVsync ? 1 : 0);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

const char* getWindowClipboard(
	Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return glfwGetClipboardString(window->handle);
}
void setWindowClipboard(
	Window window,
	const char* clipboard)
{
	assert(window);
	assert(clipboard);
	assert(graphicsInitialized);

	glfwSetClipboardString(
		window->handle,
		clipboard);
}

Vec2I getWindowSize(
	Window window)
{
	assert(window);
	assert(graphicsInitialized);

	int width, height;

	glfwGetWindowSize(
		window->handle,
		&width,
		&height);

	return vec2I(width, height);
}
void setWindowSize(
	Window window,
	Vec2I size)
{
	assert(window);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(graphicsInitialized);

	glfwSetWindowSize(
		window->handle,
		(int)size.x,
		(int)size.y);
}

Vec2I getWindowPosition(
	Window window)
{
	assert(window);
	assert(graphicsInitialized);

	Vec2I position;

	glfwGetWindowPos(
		window->handle,
		&position.x,
		&position.y);

	return position;
}
void setWindowPosition(
	Window window,
	Vec2I position)
{
	assert(window);
	assert(graphicsInitialized);

	glfwSetWindowPos(
		window->handle,
		position.x,
		position.y);
}

Vec2F getWindowCursorPosition(
	Window window)
{
	assert(window);
	assert(graphicsInitialized);

	double x, y;

	glfwGetCursorPos(
		window->handle,
		&x, &y);

	return vec2F(
		(cmmt_float_t)x,
		(cmmt_float_t)y);
}
void setWindowCursorPosition(
	Window window,
	Vec2F position)
{
	assert(window);
	assert(graphicsInitialized);

	glfwSetCursorPos(
		window->handle,
		(double)position.x,
		(double)position.y);
}

CursorMode getWindowCursorMode(
	Window window)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetInputMode(
		window->handle,
		GLFW_CURSOR);
}
void setWindowCursorMode(
	Window window,
	CursorMode mode)
{
	assert(window);
	assert(graphicsInitialized);

	glfwSetInputMode(
		window->handle,
		GLFW_CURSOR,
		mode);
}

CursorType getWindowCursorType(
	Window window)
{
	assert(window);
	assert(graphicsInitialized);
	return window->cursorType;
}
void setWindowCursorType(
	Window window,
	CursorType type)
{
	assert(window);
	assert(type < CURSOR_TYPE_COUNT);
	assert(graphicsInitialized);

	switch (type)
	{
	default:
		abort();
	case DEFAULT_CURSOR_TYPE:
		glfwSetCursor(
			window->handle,
			NULL);
		break;
	case IBEAM_CURSOR_TYPE:
		glfwSetCursor(
			window->handle,
			window->ibeamCursor);
		break;
	case CROSSHAIR_CURSOR_TYPE:
		glfwSetCursor(
			window->handle,
			window->crosshairCursor);
		break;
	case HAND_CURSOR_TYPE:
		glfwSetCursor(
			window->handle,
			window->handCursor);
		break;
	case HRESIZE_CURSOR_TYPE:
		glfwSetCursor(
			window->handle,
			window->hresizeCursor);
		break;
	case VRESIZE_CURSOR_TYPE:
		glfwSetCursor(
			window->handle,
			window->vresizeCursor);
		break;
	}
}

void setWindowTitle(
	Window window,
	const char* title)
{
	assert(window);
	assert(title);
	assert(graphicsInitialized);
	glfwSetWindowTitle(window->handle, title);
}

bool isWindowFocused(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_FOCUSED);
}
bool isWindowIconified(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_ICONIFIED);
}
bool isWindowMaximized(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_MAXIMIZED);
}
bool isWindowVisible(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_VISIBLE);
}
bool isWindowHovered(Window window)
{
	assert(window);
	assert(graphicsInitialized);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_HOVERED);
}

void iconifyWindow(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	glfwIconifyWindow(window->handle);
}
void maximizeWindow(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	glfwMaximizeWindow(window->handle);
}
void restoreWindow(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	glfwRestoreWindow(window->handle);
}
void showWindow(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	glfwShowWindow(window->handle);
}
void hideWindow(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	glfwHideWindow(window->handle);
}
void focusWindow(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	glfwFocusWindow(window->handle);
}
void requestWindowAttention(Window window)
{
	assert(window);
	assert(graphicsInitialized);
	glfwRequestWindowAttention(window->handle);
}

void makeGlWindowContextCurrent(Window window)
{
	assert(window);
	assert(graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API);
	assert(!window->isRecording);
	assert(graphicsInitialized);

#if MPGX_SUPPORT_OPENGL
	if (window != currentWindow)
	{
		glfwMakeContextCurrent(window->handle);
		currentWindow = window;
	}
#else
	abort();
#endif
}
void joinWindow(Window window)
{
	assert(window);
	assert(window->isRecording == false);
	assert(graphicsInitialized);

	GLFWwindow* handle = window->handle;
	OnWindowUpdate onUpdate = window->onUpdate;
	void* updateArgument = window->updateArgument;

	while (!glfwWindowShouldClose(handle))
	{
		window->inputLength = 0;
		glfwPollEvents();

		double startTime = getCurrentClock();
		window->deltaTime = startTime - window->updateTime;
		window->updateTime = startTime;
		onUpdate(updateArgument);
	}
}

MpgxResult beginWindowRecord(Window window)
{
	assert(window);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	int width, height;

	glfwGetFramebufferSize(
		window->handle,
		&width,
		&height);

	if (width <= 0 || height <= 0)
		return ZERO_FRAMEBUFFER_SIZE_MPGX_RESULT;

	Vec2I newSize = vec2I(width, height);
	Framebuffer framebuffer = window->framebuffer;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VmaAllocator allocator = vkWindow->allocator;

		if (vkWindow->stagingBuffer)
		{
			vmaDestroyBuffer(
				vkWindow->allocator,
				vkWindow->stagingBuffer,
				vkWindow->stagingAllocation);
			vkWindow->stagingBuffer = NULL;
			vkWindow->stagingAllocation = NULL;
			vkWindow->stagingSize = 0;
		}

		if (!compVec2I(newSize, framebuffer->vk.size))
		{
			MpgxResult mpgxResult = onVkResize(window, newSize);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
				return mpgxResult;
		}

		VkDevice device = vkWindow->device;
		uint32_t frameIndex = vkWindow->frameIndex;
		VkFence fence = vkWindow->fences[frameIndex];

		VkResult vkResult = vkWaitForFences(
			device,
			1,
			&fence,
			VK_TRUE,
			UINT64_MAX);

		if (vkResult != VK_SUCCESS)
			return vkToMpgxResult(vkResult);

		vkResult = vkResetFences(
			device,
			1,
			&fence);

		if (vkResult != VK_SUCCESS)
			return vkToMpgxResult(vkResult);

		VkSemaphore* imageAcquiredSemaphores =
			vkWindow->imageAcquiredSemaphores;
		VkSwapchain swapchain = vkWindow->swapchain;
		VkSwapchainKHR handle = swapchain->handle;

		uint32_t bufferIndex;

		do
		{
			vkResult = vkAcquireNextImageKHR(
				device,
				handle,
				UINT64_MAX,
				imageAcquiredSemaphores[frameIndex],
				NULL,
				&bufferIndex);

			if (vkResult == VK_ERROR_OUT_OF_DATE_KHR)
			{
				MpgxResult mpgxResult = onVkResize(window, newSize);

				if (mpgxResult != SUCCESS_MPGX_RESULT)
					return mpgxResult;
			}
			else if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			{
				return OUT_OF_HOST_MEMORY_MPGX_RESULT;
			}
			else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			{
				return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
			}
			else if (vkResult == VK_ERROR_DEVICE_LOST)
			{
				return DEVICE_IS_LOST_MPGX_RESULT;
			}
			else if (vkResult == VK_ERROR_SURFACE_LOST_KHR)
			{
				return SURFACE_IS_LOST_MPGX_RESULT;
			}
		} while (vkResult != VK_SUCCESS);

		vmaSetCurrentFrameIndex(
			allocator,
			bufferIndex);

		VkSwapchainBuffer buffer = swapchain->buffers[bufferIndex];
		VkCommandBuffer graphicsCommandBuffer = buffer.graphicsCommandBuffer;
		framebuffer->vk.renderPass = swapchain->renderPass;
		framebuffer->vk.handle = buffer.framebuffer;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			NULL,
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			NULL,
		};

		vkResult = vkBeginCommandBuffer(
			graphicsCommandBuffer,
			&commandBufferBeginInfo);

		if (vkResult != VK_SUCCESS)
			return vkToMpgxResult(vkResult);

		vkWindow->bufferIndex = bufferIndex;
		vkWindow->currenCommandBuffer = graphicsCommandBuffer;
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		if (!compVec2I(newSize, framebuffer->gl.size))
		{
			framebuffer->gl.size = newSize;

			GraphicsPipeline* graphicsPipelines = framebuffer->gl.graphicsPipelines;
			size_t graphicsPipelineCount = framebuffer->gl.graphicsPipelineCount;

			for (size_t i = 0; i < graphicsPipelineCount; i++)
			{
				GraphicsPipeline graphicsPipeline = graphicsPipelines[i];

				MpgxResult mpgxResult = graphicsPipeline->gl.onResize(
					graphicsPipeline,
					newSize,
					NULL);

				if (mpgxResult != SUCCESS_MPGX_RESULT)
					return mpgxResult;
			}
		}
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

#ifndef NDEBUG
	window->isRecording = true;
#endif
	return SUCCESS_MPGX_RESULT;
}

void endWindowRecord(Window window)
{
	assert(window);
	assert(window->isRecording);
	assert(!window->renderFramebuffer);
	assert(graphicsInitialized);

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VkSwapchain swapchain = vkWindow->swapchain;

		uint32_t graphicsQueueFamilyIndex =
			vkWindow->graphicsQueueFamilyIndex;
		uint32_t presentQueueFamilyIndex =
			vkWindow->presentQueueFamilyIndex;

		uint32_t bufferIndex = vkWindow->bufferIndex;

		VkSwapchainBuffer* buffer =
			&swapchain->buffers[bufferIndex];
		VkCommandBuffer graphicsCommandBuffer =
			buffer->graphicsCommandBuffer;

		if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
		{
			VkImageMemoryBarrier imageMemoryBarrier = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				NULL,
				VK_ACCESS_NONE_KHR,
				VK_ACCESS_NONE_KHR,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				graphicsQueueFamilyIndex,
				presentQueueFamilyIndex,
				buffer->image,
				{
					VK_IMAGE_ASPECT_COLOR_BIT,
					0,
					1,
					0,
					1,
				},
			};

			vkCmdPipelineBarrier(
				graphicsCommandBuffer,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,
				0,
				NULL,
				0,
				NULL,
				1,
				&imageMemoryBarrier);
		}

		VkResult vkResult = vkEndCommandBuffer(
			graphicsCommandBuffer);

		if (vkResult != VK_SUCCESS)
			abort();

		uint32_t frameIndex = vkWindow->frameIndex;

		VkSemaphore drawCompleteSemaphore =
			vkWindow->drawCompleteSemaphores[frameIndex];

		VkPipelineStageFlags pipelineStage =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			NULL,
			1,
			&vkWindow->imageAcquiredSemaphores[frameIndex],
			&pipelineStage,
			1,
			&graphicsCommandBuffer,
			1,
			&drawCompleteSemaphore,
		};

		vkResult = vkQueueSubmit(
			vkWindow->graphicsQueue,
			1,
			&submitInfo,
			vkWindow->fences[frameIndex]);

		if (vkResult != VK_SUCCESS)
			abort();

		VkSwapchainKHR handle = swapchain->handle;

		VkPresentInfoKHR presentInfo = {
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			NULL,
			1,
			&drawCompleteSemaphore,
			1,
			&handle,
			&bufferIndex,
			NULL,
		};

		VkSemaphore imageOwnershipSemaphore =
			vkWindow->imageOwnershipSemaphores[frameIndex];
		VkQueue presentQueue = vkWindow->presentQueue;

		if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
		{
			submitInfo.pWaitSemaphores = &drawCompleteSemaphore;
			submitInfo.pCommandBuffers = &buffer->presentCommandBuffer;
			submitInfo.pSignalSemaphores = &imageOwnershipSemaphore;

			vkResult = vkQueueSubmit(
				presentQueue,
				1,
				&submitInfo,
				NULL);

			if (vkResult != VK_SUCCESS)
				abort();

			presentInfo.pWaitSemaphores =
				&imageOwnershipSemaphore;
		}

		vkResult = vkQueuePresentKHR(
			presentQueue,
			&presentInfo);

		vkWindow->frameIndex =
			(frameIndex + 1) % VK_FRAME_LAG;

		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			int width, height;

			glfwGetFramebufferSize(
				window->handle,
				&width,
				&height);

			if (width <= 0 || height <= 0)
				return;

			Vec2I newSize = vec2I(width, height);
			MpgxResult mpgxResult = onVkResize(window, newSize);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
				abort();
		}
		else if (vkResult != VK_SUCCESS &&
			vkResult != VK_SUBOPTIMAL_KHR &&
			vkResult == VK_ERROR_SURFACE_LOST_KHR)
		{
			abort();
		}
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		glfwSwapBuffers(window->handle);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

#ifndef NDEBUG
	window->isRecording = false;
#endif
}

MpgxResult createBuffer(
	Window window,
	BufferType type,
	BufferUsage usage,
	const void* data,
	size_t size,
	Buffer* buffer)
{
	assert(window);
	assert(type > 0);
	assert(usage < BUFFER_USAGE_COUNT);
	assert(size > 0);
	assert(buffer);
	assert(graphicsInitialized);

	assert((usage != GPU_TO_CPU_BUFFER_USAGE) ||
		(usage == GPU_TO_CPU_BUFFER_USAGE && !data));
	assert(!window->isRecording);

	MpgxResult mpgxResult;
	Buffer bufferInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		mpgxResult = createVkBuffer(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->transferQueue,
			vkWindow->transferCommandBuffer,
			vkWindow->transferFence,
			&vkWindow->stagingBuffer,
			&vkWindow->stagingAllocation,
			&vkWindow->stagingSize,
			window,
			type,
			usage,
			data,
			size,
			window->useRayTracing,
			&bufferInstance);
#else
	abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		mpgxResult = createGlBuffer(
			window,
			type,
			usage,
			data,
			size,
			&bufferInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = window->bufferCount;

	if (count == window->bufferCapacity)
	{
		size_t capacity = window->bufferCapacity * 2;

		Buffer* buffers = realloc(
			window->buffers,
			sizeof(Buffer) * capacity);

		if (!buffers)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkBuffer(
					window->vkWindow->allocator,
					bufferInstance);
#else
				abort();
#endif
			}
			else
			{
#if MPGX_SUPPORT_OPENGL
				destroyGlBuffer(bufferInstance);
#else
				abort();
#endif
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		window->buffers = buffers;
		window->bufferCapacity = capacity;
	}

	window->buffers[count] = bufferInstance;
	window->bufferCount = count + 1;

	*buffer = bufferInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyBuffer(Buffer buffer)
{
	if (!buffer)
		return;

	assert(!buffer->base.isMapped);
	assert(!buffer->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = buffer->base.window;
	Buffer* buffers = window->buffers;
	size_t bufferCount = window->bufferCount;

	for (size_t i = 0; i < bufferCount; i++)
	{
		if (buffer != buffers[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkBuffer(
				vkWindow->allocator,
				buffer);
#else
			abort();
#endif
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlBuffer(buffer);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < bufferCount; j++)
			buffers[j - 1] = buffers[j];

		window->bufferCount--;
		return;
	}

	abort();
}

Window getBufferWindow(Buffer buffer)
{
	assert(buffer);
	assert(graphicsInitialized);
	return buffer->base.window;
}
BufferType getBufferType(Buffer buffer)
{
	assert(buffer);
	assert(graphicsInitialized);
	return buffer->base.type;
}
BufferUsage getBufferUsage(Buffer buffer)
{
	assert(buffer);
	assert(graphicsInitialized);
	return buffer->base.usage;
}
size_t getBufferSize(Buffer buffer)
{
	assert(buffer);
	assert(graphicsInitialized);
	return buffer->base.size;
}

MpgxResult mapBuffer(
	Buffer buffer,
	size_t size,
	size_t offset,
	void** map)
{
	assert(buffer);
	assert(map);
	assert(graphicsInitialized);

	assert((size == 0 && offset == 0) ||
		(size > 0 && size + offset <= buffer->base.size));
	assert(buffer->base.usage == CPU_ONLY_BUFFER_USAGE ||
		buffer->base.usage == CPU_TO_GPU_BUFFER_USAGE ||
		buffer->base.usage == GPU_TO_CPU_BUFFER_USAGE);
	assert(!buffer->base.isMapped);

	size_t mapSize = size > 0 ? size : buffer->base.size;
	Window window = buffer->base.window;

	MpgxResult mpgxResult;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		mpgxResult = mapVkBuffer(
			window->vkWindow->allocator,
			buffer->vk.allocation,
			buffer->vk.usage,
			mapSize,
			offset,
			map);

		buffer->vk.mapSize = mapSize;
		buffer->vk.mapOffset = offset;
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		mpgxResult = mapGlBuffer(
			buffer->gl.glType,
			buffer->gl.handle,
			buffer->gl.usage,
			mapSize,
			offset,
			map);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

#ifndef NDEBUG
	buffer->base.isMapped = true;
#endif
	return SUCCESS_MPGX_RESULT;
}
MpgxResult unmapBuffer(Buffer buffer)
{
	assert(buffer);
	assert(buffer->base.isMapped);
	assert(graphicsInitialized);

	Window window = buffer->base.window;

	MpgxResult mpgxResult;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		mpgxResult = unmapVkBuffer(
			window->vkWindow->allocator,
			buffer->vk.allocation,
			buffer->vk.usage,
			buffer->vk.mapSize,
			buffer->vk.mapOffset);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		mpgxResult = unmapGlBuffer(
			buffer->gl.glType,
			buffer->gl.handle);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

#ifndef NDEBUG
	buffer->base.isMapped = false;
#endif
	return mpgxResult;
}

MpgxResult setBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	assert(buffer);
	assert(data);
	assert(size > 0);
	assert(buffer->base.usage == CPU_ONLY_BUFFER_USAGE ||
		buffer->base.usage == CPU_TO_GPU_BUFFER_USAGE);
	assert(!buffer->base.isMapped);
	assert(size + offset <= buffer->base.size);
	assert(graphicsInitialized);

	Window window = buffer->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return setVkBufferData(
			window->vkWindow->allocator,
			buffer->vk.allocation,
			data,
			size,
			offset);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return setGlBufferData(
			buffer->gl.glType,
			buffer->gl.handle,
			data,
			size,
			offset);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

MpgxResult createMipmapImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3I size,
	uint32_t mipCount,
	uint32_t layerCount,
	bool isConstant,
	Image* image)
{
	assert(window);
	assert(type > 0);
	assert(dimension < IMAGE_DIMENSION_COUNT);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(data);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(mipCount > 0);
	assert(layerCount > 0);
	assert(image);
	assert(!window->isRecording);
	assert(mipCount <= calcMipLevelCount(size));
	assert(graphicsInitialized);

	MpgxResult mpgxResult;
	Image imageInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		mpgxResult = createVkImage(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->transferQueue,
			vkWindow->transferCommandBuffer,
			vkWindow->transferFence,
			&vkWindow->stagingBuffer,
			&vkWindow->stagingAllocation,
			&vkWindow->stagingSize,
			window,
			type,
			dimension,
			format,
			data,
			size,
			mipCount,
			layerCount,
			isConstant,
			&imageInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		if (layerCount != 1)
			return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;

		mpgxResult = createGlImage(
			window,
			type,
			dimension,
			format,
			data,
			size,
			mipCount,
			isConstant,
			&imageInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = window->imageCount;

	if (count == window->imageCapacity)
	{
		size_t capacity = window->imageCapacity * 2;

		Image* images = realloc(
			window->images,
			sizeof(Image) * capacity);

		if (!images)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkImage(
					vkWindow->device,
					vkWindow->allocator,
					imageInstance);
#else
				abort();
#endif
			}
			else
			{
#if MPGX_SUPPORT_OPENGL
				destroyGlImage(imageInstance);
#else
				abort();
#endif
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		window->images = images;
		window->imageCapacity = capacity;
	}

	window->images[count] = imageInstance;
	window->imageCount = count + 1;

	*image = imageInstance;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void* data,
	Vec3I size,
	uint32_t layerCount,
	bool isConstant,
	Image* image)
{
	assert(window);
	assert(type > 0);
	assert(dimension < IMAGE_DIMENSION_COUNT);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(data);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(layerCount > 0);
	assert(image);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	return createMipmapImage(
		window,
		type,
		dimension,
		format,
		&data,
		size,
		1,
		layerCount,
		isConstant,
		image);
}
void destroyImage(Image image)
{
	if (!image)
		return;

	assert(!image->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = image->base.window;
	Image* images = window->images;
	size_t imageCount = window->imageCount;

	for (size_t i = 0; i < imageCount; i++)
	{
		if (image != images[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkImage(
				vkWindow->device,
				vkWindow->allocator,
				image);
#else
			abort();
#endif
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlImage(image);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < imageCount; j++)
			images[j - 1] = images[j];

		window->imageCount--;
		return;
	}

	abort();
}

MpgxResult setMipmapImageData(
	Image image,
	const void** data,
	Vec3I size,
	Vec3I offset)
{
	// TODO:
	abort();
}
MpgxResult setImageData(
	Image image,
	const void* data,
	Vec3I size,
	Vec3I offset,
	uint8_t mipLevel)
{
	assert(image);
	assert(data);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(offset.x >= 0);
	assert(offset.y >= 0);
	assert(offset.z >= 0);
	assert(size.x + offset.x <= image->base.size.x);
	assert(size.y + offset.y <= image->base.size.y);
	assert(size.z + offset.z <= image->base.size.z);
	assert(!image->base.isConstant);
	assert(!image->base.window->isRecording);
	assert(mipLevel < image->base.mipCount);
	assert(graphicsInitialized);

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow =
			image->vk.window->vkWindow;

		return setVkImageData(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->transferQueue,
			vkWindow->transferCommandBuffer,
			vkWindow->transferFence,
			image->vk.stagingBuffer,
			image->vk.stagingAllocation,
			image,
			data,
			size,
			offset,
			mipLevel);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return setGlImageData(
			image,
			data,
			size,
			offset,
			mipLevel);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

Window getImageWindow(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.window;
}
ImageType getImageType(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.type;
}
ImageDimension getImageDimension(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.dimension;
}
ImageFormat getImageFormat(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.format;
}
uint32_t getImageMipCount(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.mipCount;
}
uint32_t getImageLayerCount(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.layerCount;
}
Vec3I getImageSize(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.size;
}
bool isImageConstant(Image image)
{
	assert(image);
	assert(graphicsInitialized);
	return image->base.isConstant;
}

MpgxResult createSampler(
	Window window,
	ImageFilter minImageFilter,
	ImageFilter magImageFilter,
	ImageFilter minMipmapFilter,
	bool useMipmapping,
	ImageWrap imageWrapX,
	ImageWrap imageWrapY,
	ImageWrap imageWrapZ,
	CompareOperator depthCompare,
	bool useCompare,
	Vec2F mipmapLodRange,
	float mipmapLodBias,
	Sampler* sampler)
{
	assert(window);
	assert(minImageFilter < IMAGE_FILTER_COUNT);
	assert(magImageFilter < IMAGE_FILTER_COUNT);
	assert(minMipmapFilter < IMAGE_FILTER_COUNT);
	assert(imageWrapX < IMAGE_WRAP_COUNT);
	assert(imageWrapY < IMAGE_WRAP_COUNT);
	assert(imageWrapZ < IMAGE_WRAP_COUNT);
	assert(depthCompare < COMPARE_OPERATOR_COUNT);
	assert(sampler);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	MpgxResult mpgxResult;
	Sampler samplerInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		mpgxResult = createVkSampler(
			window->vkWindow->device,
			window,
			minImageFilter,
			magImageFilter,
			minMipmapFilter,
			useMipmapping,
			imageWrapX,
			imageWrapY,
			imageWrapZ,
			depthCompare,
			useCompare,
			mipmapLodRange,
			mipmapLodBias,
			&samplerInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		if (mipmapLodBias != 0.0f)
			return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;

		mpgxResult = createGlSampler(
			window,
			minImageFilter,
			magImageFilter,
			minMipmapFilter,
			useMipmapping,
			imageWrapX,
			imageWrapY,
			imageWrapZ,
			depthCompare,
			useCompare,
			mipmapLodRange,
			&samplerInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = window->samplerCount;

	if (count == window->samplerCapacity)
	{
		size_t capacity = window->samplerCapacity * 2;

		Sampler* samplers = realloc(
			window->samplers,
			sizeof(Sampler) * capacity);

		if (!samplers)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkSampler(
					window->vkWindow->device,
					samplerInstance);
#else
				abort();
#endif
			}
			else
			{
#if MPGX_SUPPORT_OPENGL
				destroyGlSampler(samplerInstance);
#else
				abort();
#endif
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		window->samplers = samplers;
		window->samplerCapacity = capacity;
	}

	window->samplers[count] = samplerInstance;
	window->samplerCount = count + 1;

	*sampler = samplerInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroySampler(Sampler sampler)
{
	if (!sampler)
		return;

	assert(!sampler->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = sampler->base.window;
	Sampler* samplers = window->samplers;
	size_t samplerCount = window->samplerCount;

	for (size_t i = 0; i < samplerCount; i++)
	{
		if (sampler != samplers[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkSampler(
				vkWindow->device,
				sampler);
#else
			abort();
#endif
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlSampler(sampler);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < samplerCount; j++)
			samplers[j - 1] = samplers[j];

		window->samplerCount--;
		return;
	}

	abort();
}

Window getSamplerWindow(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.window;
}
ImageFilter getSamplerMinImageFilter(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.minImageFilter;
}
ImageFilter getSamplerMagImageFilter(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.magImageFilter;
}
ImageFilter getSamplerMinMipmapFilter(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.minMipmapFilter;
}
bool isSamplerUseMipmapping(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.useMipmapping;
}
ImageWrap getSamplerImageWrapX(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.imageWrapX;
}
ImageWrap getSamplerImageWrapY(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.imageWrapY;
}
ImageWrap getSamplerImageWrapZ(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.imageWrapZ;
}
CompareOperator getSamplerDepthCompare(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.depthCompare;
}
bool isSamplerUseCompare(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.useCompare;
}
Vec2F getSamplerMipmapLodRange(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.mipmapLodRange;
}
float getSamplerMipmapLodBias(Sampler sampler)
{
	assert(sampler);
	assert(graphicsInitialized);
	return sampler->base.mipmapLodBias;
}

MpgxResult createShader(
	Window window,
	ShaderType type,
	const void* code,
	size_t size,
	Shader* shader)
{
	assert(window);
	assert(type < SHADER_TYPE_COUNT);
	assert(code);
	assert(size > 0);
	assert(shader);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	MpgxResult mpgxResult;
	Shader shaderInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		mpgxResult = createVkShader(
			window->vkWindow->device,
			window,
			type,
			code,
			size,
			&shaderInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		mpgxResult = createGlShader(
			window,
			type,
			code,
			size,
			graphicsAPI,
			&shaderInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = window->shaderCount;

#ifndef NDEBUG
	MD5_CONTEXT md5Context;
	md5_init(&md5Context);

	md5_update(
		&md5Context,
		code,
		size);

	uint8_t* hash = shaderInstance->base.hash;
	md5_final(&md5Context, hash);

	Shader* windowShaders = window->shaders;

	for (size_t i = 0; i < count; i++)
	{
		Shader otherShader = windowShaders[i];

		int result = memcmp(
			hash,
			otherShader->base.hash,
			MD5_BLOCK_SIZE);

		if (result == 0)
		{
			printf("Shader %p have same code with %p.\n",
				shader, otherShader);
		}
	}
#endif

	if (count == window->shaderCapacity)
	{
		size_t capacity = window->shaderCapacity * 2;

		Shader* shaders = realloc(
			window->shaders,
			sizeof(Shader) * capacity);

		if (!shaders)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkShader(
					window->vkWindow->device,
					shaderInstance);
#else
				abort();
#endif
			}
			else
			{
#if MPGX_SUPPORT_OPENGL
				destroyGlShader(shaderInstance);
#else
				abort();
#endif
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		window->shaders = shaders;
		window->shaderCapacity = capacity;
	}

	window->shaders[count] = shaderInstance;
	window->shaderCount = count + 1;

	*shader = shaderInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyShader(Shader shader)
{
	if (!shader)
		return;

	assert(!shader->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = shader->base.window;
	Shader* shaders = window->shaders;
	size_t shaderCount = window->shaderCount;

	for (size_t i = 0; i < shaderCount; i++)
	{
		if (shader != shaders[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkShader(
				window->vkWindow->device,
				shader);
#else
			abort();
#endif
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlShader(shader);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < shaderCount; j++)
			shaders[j - 1] = shaders[j];

		window->shaderCount--;
		return;
	}

	abort();
}

Window getShaderWindow(Shader shader)
{
	assert(shader);
	assert(graphicsInitialized);
	return shader->base.window;
}
ShaderType getShaderType(Shader shader)
{
	assert(shader);
	assert(graphicsInitialized);
	return shader->base.type;
}

inline static bool addWindowFramebuffer(
	Window window,
	Framebuffer framebuffer)
{
	assert(window);
	assert(framebuffer);

	assert(window);
	assert(framebuffer);

	size_t count = window->framebufferCount;

	if (count == window->framebufferCapacity)
	{
		size_t capacity = window->framebufferCapacity * 2;

		Framebuffer* framebuffers = realloc(
			window->framebuffers,
			sizeof(Framebuffer) * capacity);

		if (!framebuffers)
			return false;

		window->framebuffers = framebuffers;
		window->framebufferCapacity = capacity;
	}

	window->framebuffers[count] = framebuffer;
	window->framebufferCount = count + 1;
	return true;
}
MpgxResult createFramebuffer(
	Window window,
	Vec2I size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t capacity,
	Framebuffer* framebuffer)
{
	assert(window);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(capacity > 0);
	assert(framebuffer);
	assert(!window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	bool hasSomeAttachments = false;

	if (colorAttachmentCount > 0)
	{
		assert(colorAttachments);

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image image = colorAttachments[i];

			assert(image->base.type &
				COLOR_ATTACHMENT_IMAGE_TYPE);
			assert(image->base.size.x  == size.x &&
				image->base.size.y == size.y);
			assert(image->base.window == window);
		}

		hasSomeAttachments = true;
	}
	else
	{
		assert(!colorAttachments);
	}
	if (depthStencilAttachment)
	{
		assert(depthStencilAttachment->base.type &
			DEPTH_STENCIL_ATTACHMENT_IMAGE_TYPE);
		assert(depthStencilAttachment->base.size.x == size.x &&
			depthStencilAttachment->base.size.y == size.y);
		assert(depthStencilAttachment->base.window == window);
		hasSomeAttachments = true;
	}

	assert(hasSomeAttachments);
#endif

	MpgxResult mpgxResult;
	Framebuffer framebufferInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VkDevice device = vkWindow->device;

		VkRenderPass renderPass;

		mpgxResult = createVkGeneralRenderPass(
			device,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment,
			&renderPass);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		mpgxResult = createVkFramebuffer(
			device,
			renderPass,
			window,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment,
			capacity,
			&framebufferInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		mpgxResult = createGlFramebuffer(
			window,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment,
			capacity,
			&framebufferInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	if (!addWindowFramebuffer(window, framebufferInstance))
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(
				window->vkWindow->device,
				framebufferInstance);
#else
			abort();
#endif
		}
		else
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlFramebuffer(framebufferInstance);
#else
			abort();
#endif
		}

		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*framebuffer = framebufferInstance;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createShadowFramebuffer(
	Window window,
	Vec2I size,
	bool useBeginClear,
	Image depthAttachment,
	size_t capacity,
	Framebuffer* framebuffer)
{
	assert(window);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(depthAttachment);
	assert(depthAttachment->base.size.x == size.x &&
		depthAttachment->base.size.y == size.y);
	assert(depthAttachment->base.window == window);
	assert(capacity > 0);
	assert(framebuffer);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	MpgxResult mpgxResult;
	Framebuffer framebufferInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VkDevice device = vkWindow->device;

		VkRenderPass renderPass;

		mpgxResult = createVkShadowRenderPass(
			device,
			depthAttachment->vk.vkFormat,
			useBeginClear,
			&renderPass);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		mpgxResult = createVkFramebuffer(
			device,
			renderPass,
			window,
			size,
			useBeginClear,
			NULL,
			0,
			depthAttachment,
			capacity,
			&framebufferInstance);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			vkDestroyRenderPass(
				device,
				renderPass,
				NULL);
		}
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		mpgxResult = createGlFramebuffer(
			window,
			size,
			useBeginClear,
			NULL,
			0,
			depthAttachment,
			capacity,
			&framebufferInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	if (!addWindowFramebuffer(window, framebufferInstance))
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(
				window->vkWindow->device,
				framebufferInstance);
#else
			abort();
#endif
		}
		else
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlFramebuffer(framebufferInstance);
#else
			abort();
#endif
		}

		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*framebuffer = framebufferInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyFramebuffer(Framebuffer framebuffer)
{
	if (!framebuffer)
		return;

	assert(!framebuffer->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = framebuffer->base.window;
	Framebuffer* framebuffers = window->framebuffers;
	size_t framebufferCount = window->framebufferCount;

	for (size_t i = 0; i < framebufferCount; i++)
	{
		if (framebuffer != framebuffers[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkFramebuffer(
				vkWindow->device,
				framebuffer);
#else
			abort();
#endif
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlFramebuffer(framebuffer);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < framebufferCount; j++)
			framebuffers[j - 1] = framebuffers[j];

		window->framebufferCount--;
		return;
	}

	abort();
}

Window getFramebufferWindow(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(graphicsInitialized);
	return framebuffer->base.window;
}
Vec2I getFramebufferSize(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(graphicsInitialized);
	return framebuffer->base.size;
}
bool isFramebufferUseBeginClear(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(graphicsInitialized);
	return framebuffer->base.useBeginClear;
}
Image* getFramebufferColorAttachments(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(!framebuffer->base.isDefault);
	assert(graphicsInitialized);
	return framebuffer->base.colorAttachments;
}
size_t getFramebufferColorAttachmentCount(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(!framebuffer->base.isDefault);
	assert(graphicsInitialized);
	return framebuffer->base.colorAttachmentCount;
}
Image getFramebufferDepthStencilAttachment(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(!framebuffer->base.isDefault);
	assert(graphicsInitialized);
	return framebuffer->base.depthStencilAttachment;
}
bool isFramebufferDefault(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(graphicsInitialized);
	return framebuffer->base.isDefault;
}

MpgxResult setFramebufferAttachments(
	Framebuffer framebuffer,
	Vec2I size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	assert(framebuffer);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(!framebuffer->base.isDefault);
	assert(!framebuffer->base.window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	bool hasSomeAttachments = false;

	if (colorAttachmentCount > 0)
	{
		assert(colorAttachments);

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image image = colorAttachments[i];

			assert(image->base.type &
				   COLOR_ATTACHMENT_IMAGE_TYPE);
			assert(image->base.size.x  == size.x &&
				   image->base.size.y == size.y);
			assert(image->base.window == framebuffer->base.window);
		}

		hasSomeAttachments = true;
	}
	else
	{
		assert(!colorAttachments);
	}
	if (depthStencilAttachment)
	{
		assert(depthStencilAttachment->base.type &
			   DEPTH_STENCIL_ATTACHMENT_IMAGE_TYPE);
		assert(depthStencilAttachment->base.size.x == size.x &&
			   depthStencilAttachment->base.size.y == size.y);
		assert(depthStencilAttachment->base.window == framebuffer->base.window);
		hasSomeAttachments = true;
	}

	assert(hasSomeAttachments);
#endif

	Window window = framebuffer->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		VkResult vkResult = vkQueueWaitIdle(
			vkWindow->graphicsQueue);

		if (vkResult != VK_SUCCESS)
			return vkToMpgxResult(vkResult);

		VkDevice device = vkWindow->device;

		VkRenderPass renderPass;

		MpgxResult mpgxResult = createVkGeneralRenderPass(
			device,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment,
			&renderPass);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		mpgxResult = setVkFramebufferAttachments(
			vkWindow->device,
			renderPass,
			framebuffer,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			vkDestroyRenderPass(
				device,
				renderPass,
				NULL);
			return mpgxResult;
		}

		return SUCCESS_MPGX_RESULT;
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return setGlFramebufferAttachments(
			framebuffer,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

void beginFramebufferRender(
	Framebuffer framebuffer,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	assert(framebuffer);
	assert((clearValues && clearValueCount > 0 &&
			framebuffer->base.useBeginClear) ||
		   (!clearValues && clearValueCount == 0 &&
			!framebuffer->base.useBeginClear));
	assert(framebuffer->base.window->isRecording);
	assert(!framebuffer->base.window->renderFramebuffer);
	assert(graphicsInitialized);

#ifndef NDEBUG
	if (clearValueCount > 0)
	{
		Image depthStencilAttachment =
			framebuffer->base.depthStencilAttachment;
		size_t colorAttachmentCount =
			framebuffer->base.colorAttachmentCount;
		size_t attachmentCount = depthStencilAttachment ?
								 colorAttachmentCount + 1 : colorAttachmentCount;

		if (framebuffer->base.isDefault)
			assert(clearValueCount == 2);
		else
			assert(clearValueCount == attachmentCount);

		if (depthStencilAttachment)
		{
			float depth = clearValues[colorAttachmentCount].depthStencil.depth;
			assert(depth >= 0.0f && depth <= 1.0f);
		}
	}
#endif

	Window window = framebuffer->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		beginVkFramebufferRender(
			vkWindow->currenCommandBuffer,
			framebuffer->vk.renderPass,
			framebuffer->vk.handle,
			framebuffer->vk.size,
			clearValues,
			clearValueCount);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		bool hasDepthBuffer, hasStencilBuffer;

		if (framebuffer->gl.isDefault)
		{
			hasDepthBuffer = true;
			hasStencilBuffer = window->useStencilBuffer;
		}
		else
		{
			Image depthStencilAttachment =
				framebuffer->gl.depthStencilAttachment;

			if (depthStencilAttachment)
			{
				ImageFormat format = depthStencilAttachment->gl.format;

				switch (format)
				{
				default:
					abort();
				case D16_UNORM_IMAGE_FORMAT:
				case D32_SFLOAT_IMAGE_FORMAT:
					hasDepthBuffer = true;
					hasStencilBuffer = false;
					break;
				case D16_UNORM_S8_UINT_IMAGE_FORMAT:
				case D24_UNORM_S8_UINT_IMAGE_FORMAT:
				case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
					hasDepthBuffer = true;
					hasStencilBuffer = true;
					break;
				}
			}
			else
			{
				hasDepthBuffer = false;
				hasStencilBuffer = false;
			}
		}

		bool hasDepthStencilAttachment =
			framebuffer->gl.isDefault ||
			framebuffer->gl.depthStencilAttachment;

		beginGlFramebufferRender(
			framebuffer->gl.handle,
			framebuffer->gl.size,
			framebuffer->gl.colorAttachmentCount,
			hasDepthBuffer,
			hasStencilBuffer,
			clearValues,
			clearValueCount);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	window->renderFramebuffer = framebuffer;
}

void endFramebufferRender(Framebuffer framebuffer)
{
	assert(framebuffer);
	assert(framebuffer->base.window->isRecording);
	assert(framebuffer->base.window->renderFramebuffer == framebuffer);
	assert(graphicsInitialized);

	Window window = framebuffer->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		endVkFramebufferRender(
			window->vkWindow->currenCommandBuffer);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		endGlFramebufferRender();
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	window->renderFramebuffer = NULL;
}

void clearFramebuffer(
	Framebuffer framebuffer,
	const bool* clearAttachments,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	assert(framebuffer);
	assert(clearAttachments);
	assert(clearValues);
	assert(clearValueCount > 0);
	assert(framebuffer->base.window->isRecording);
	assert(framebuffer->base.window->renderFramebuffer);
	assert(graphicsInitialized);

#ifndef NDEBUG
	if (clearValueCount > 0)
	{
		Image depthStencilAttachment =
			framebuffer->base.depthStencilAttachment;
		size_t colorAttachmentCount =
			framebuffer->base.colorAttachmentCount;
		size_t attachmentCount = depthStencilAttachment ?
			colorAttachmentCount + 1 : colorAttachmentCount;

		if (framebuffer->base.isDefault)
			assert(clearValueCount == 2);
		else
			assert(clearValueCount == attachmentCount);

		if (depthStencilAttachment)
		{
			float depth = clearValues[colorAttachmentCount].depthStencil.depth;
			assert(depth >= 0.0f && depth <= 1.0f);
		}
	}
#endif

	Window window = framebuffer->base.window;
	bool hasDepthBuffer, hasStencilBuffer;

	if (framebuffer->base.isDefault)
	{
		hasDepthBuffer = true;
		hasStencilBuffer = window->useStencilBuffer;
	}
	else
	{
		Image depthStencilAttachment =
			framebuffer->base.depthStencilAttachment;

		if (depthStencilAttachment)
		{
			ImageFormat format = depthStencilAttachment->base.format;

			switch (format)
			{
			default:
				abort();
			case D16_UNORM_IMAGE_FORMAT:
			case D32_SFLOAT_IMAGE_FORMAT:
				hasDepthBuffer = true;
				hasStencilBuffer = false;
				break;
			case D16_UNORM_S8_UINT_IMAGE_FORMAT:
			case D24_UNORM_S8_UINT_IMAGE_FORMAT:
			case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
				hasDepthBuffer = true;
				hasStencilBuffer = true;
				break;
			}
		}
		else
		{
			hasDepthBuffer = false;
			hasStencilBuffer = false;
		}
	}

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		clearVkFramebuffer(
			window->vkWindow->currenCommandBuffer,
			framebuffer->vk.size,
			hasDepthBuffer,
			hasStencilBuffer,
			framebuffer->vk.clearAttachments,
			clearAttachments,
			clearValues,
			clearValueCount);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		clearGlFramebuffer(
			framebuffer->gl.size,
			framebuffer->gl.colorAttachmentCount,
			hasDepthBuffer,
			hasStencilBuffer,
			clearAttachments,
			clearValues,
			clearValueCount);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

MpgxResult createGraphicsPipeline(
	Framebuffer framebuffer,
	const char* name,
	const GraphicsPipelineState* state,
	OnGraphicsPipelineBind onBind,
	OnGraphicsPipelineUniformsSet onUniformsSet,
	OnGraphicsPipelineResize onResize,
	OnGraphicsPipelineDestroy onDestroy,
	void* handle,
	const void* createData,
	Shader* shaders,
	size_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(state);
	assert(onResize);
	assert(onDestroy);
	assert(handle);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);
	assert(state->drawMode < DRAW_MODE_COUNT);
	assert(state->polygonMode < POLYGON_MODE_COUNT);
	assert(state->cullMode < CULL_MODE_COUNT);
	assert(state->depthCompareOperator < COMPARE_OPERATOR_COUNT);
	assert(state->colorComponentWriteMask <= ALL_COLOR_COMPONENT);
	assert(state->srcColorBlendFactor < BLEND_FACTOR_COUNT);
	assert(state->dstColorBlendFactor < BLEND_FACTOR_COUNT);
	assert(state->srcAlphaBlendFactor < BLEND_FACTOR_COUNT);
	assert(state->dstAlphaBlendFactor < BLEND_FACTOR_COUNT);
	assert(state->colorBlendOperator < BLEND_OPERATOR_COUNT);
	assert(state->alphaBlendOperator < BLEND_OPERATOR_COUNT);
	assert(state->lineWidth > 0.0f);
	assert(state->viewport.z >= 0);
	assert(state->viewport.w >= 0);
	assert(state->scissor.z >= 0);
	assert(state->scissor.w >= 0);
	assert(!framebuffer->base.window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader shader = shaders[i];

		assert(shader->base.type == VERTEX_SHADER_TYPE ||
			shader->base.type == FRAGMENT_SHADER_TYPE ||
			shader->base.type == TESSELLATION_CONTROL_SHADER_TYPE ||
			shader->base.type == TESSELLATION_EVALUATION_SHADER_TYPE ||
			shader->base.type == GEOMETRY_SHADER_TYPE);
		assert(shader->base.window == framebuffer->base.window);
	}
#endif

	Window window = framebuffer->base.window;

	MpgxResult mpgxResult;
	GraphicsPipeline graphicsPipelineInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		assert(createData);

		mpgxResult = createVkGraphicsPipeline(
			window->vkWindow->device,
			createData,
			framebuffer,
			window,
			name,
			*state,
			onBind,
			onUniformsSet,
			onResize,
			onDestroy,
			handle,
			shaders,
			shaderCount,
			&graphicsPipelineInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		assert(!createData);

		if (state->discardRasterizer)
			return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;

		mpgxResult = createGlGraphicsPipeline(
			framebuffer,
			window,
			name,
			*state,
			onBind,
			onUniformsSet,
			onResize,
			onDestroy,
			handle,
			shaders,
			shaderCount,
			&graphicsPipelineInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = framebuffer->base.graphicsPipelineCount;

	if (count == framebuffer->base.graphicsPipelineCapacity)
	{
		size_t capacity = framebuffer->base.graphicsPipelineCapacity * 2;

		GraphicsPipeline* graphicsPipelines = realloc(
			framebuffer->base.graphicsPipelines,
			sizeof(GraphicsPipeline) * capacity);

		if (!graphicsPipelines)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkGraphicsPipeline(
					window->vkWindow->device,
					graphicsPipelineInstance);
#else
				abort();
#endif
			}
			else
			{
#if MPGX_SUPPORT_OPENGL
				destroyGlGraphicsPipeline(graphicsPipelineInstance);
#else
				abort();
#endif
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		framebuffer->base.graphicsPipelines = graphicsPipelines;
		framebuffer->base.graphicsPipelineCapacity = capacity;
	}

	framebuffer->base.graphicsPipelines[count] = graphicsPipelineInstance;
	framebuffer->base.graphicsPipelineCount = count + 1;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyGraphicsPipeline(GraphicsPipeline pipeline)
{
	if (!pipeline)
		return;

	assert(!pipeline->base.window->isRecording);
	assert(graphicsInitialized);

	Framebuffer framebuffer = pipeline->base.framebuffer;
	Window window = framebuffer->base.window;
	size_t graphicsPipelineCount = framebuffer->base.graphicsPipelineCount;
	GraphicsPipeline* graphicsPipelines = framebuffer->base.graphicsPipelines;

	for (size_t i = 0; i < graphicsPipelineCount; i++)
	{
		if (pipeline != graphicsPipelines[i])
			continue;

		pipeline->base.onDestroy(
			window,
			pipeline->base.handle);

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkGraphicsPipeline(
				vkWindow->device,
				pipeline);
#else
			abort();
#endif
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlGraphicsPipeline(pipeline);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < graphicsPipelineCount; j++)
			graphicsPipelines[j - 1] = graphicsPipelines[j];

		framebuffer->base.graphicsPipelineCount--;
		return;
	}

	abort();
}

Framebuffer getGraphicsPipelineFramebuffer(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.framebuffer;
}
const char* getGraphicsPipelineName(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
#ifndef NDEBUG
	return pipeline->base.name;
#else
	abort();
#endif
}
const GraphicsPipelineState* getGraphicsPipelineState(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return &pipeline->base.state;
}
OnGraphicsPipelineBind getGraphicsPipelineOnBind(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onBind;
}
OnGraphicsPipelineUniformsSet getGraphicsPipelineOnUniformsSet(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onUniformsSet;
}
OnGraphicsPipelineResize getGraphicsPipelineOnResize(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onResize;
}
OnGraphicsPipelineDestroy getGraphicsPipelineOnDestroy(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onDestroy;
}
void* getGraphicsPipelineHandle(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.handle;
}
Shader* getGraphicsPipelineShaders(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.shaders;
}
size_t getGraphicsPipelineShaderCount(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.shaderCount;
}
Window getGraphicsPipelineWindow(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.window;
}

void bindGraphicsPipeline(GraphicsPipeline pipeline)
{
	assert(pipeline);
	assert(pipeline->base.window->isRecording);
	assert(pipeline->base.framebuffer ==
		pipeline->base.window->renderFramebuffer);
	assert(graphicsInitialized);

	Window window = pipeline->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bindVkGraphicsPipeline(
			window->vkWindow->currenCommandBuffer,
			pipeline);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		bindGlGraphicsPipeline(pipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

MpgxResult createGraphicsMesh(
	Window window,
	IndexType indexType,
	uint32_t indexCount,
	uint32_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	GraphicsMesh* graphicsMesh)
{
	assert(window);
	assert(graphicsMesh);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(!window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	if (vertexBuffer)
	{
		assert(vertexBuffer->base.window == window);
		assert(vertexBuffer->base.type & VERTEX_BUFFER_TYPE);
	}
	if (indexBuffer)
	{
		assert(indexBuffer->base.window == window);
		assert(indexBuffer->base.type & INDEX_BUFFER_TYPE);

		if (indexType == UINT16_INDEX_TYPE)
		{
			assert(indexCount * sizeof(uint16_t) +
				indexOffset * sizeof(uint16_t) <=
				indexBuffer->base.size);
		}
		else if (indexType == UINT32_INDEX_TYPE)
		{
			assert(indexCount * sizeof(uint32_t) +
				indexOffset * sizeof(uint32_t) <=
				indexBuffer->base.size);
		}
		else
		{
			abort();
		}
	}
#endif

	MpgxResult mpgxResult;
	GraphicsMesh graphicsMeshInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		mpgxResult = createVkGraphicsMesh(
			window,
			indexType,
			indexCount,
			indexOffset,
			vertexBuffer,
			indexBuffer,
			&graphicsMeshInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		mpgxResult = createGlGraphicsMesh(
			window,
			indexType,
			indexCount,
			indexOffset,
			vertexBuffer,
			indexBuffer,
			&graphicsMeshInstance);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = window->graphicsMeshCount;

	if (count == window->graphicsMeshCapacity)
	{
		size_t capacity = window->graphicsMeshCapacity * 2;

		GraphicsMesh* graphicsMeshes = realloc(
			window->graphicsMeshes,
			sizeof(GraphicsMesh) * capacity);

		if (!graphicsMeshes)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkGraphicsMesh(graphicsMeshInstance);
#else
				abort();
#endif
			}
			else
			{
#if MPGX_SUPPORT_OPENGL
				destroyGlGraphicsMesh(graphicsMeshInstance);
#else
				abort();
#endif
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		window->graphicsMeshes = graphicsMeshes;
		window->graphicsMeshCapacity = capacity;
	}

	window->graphicsMeshes[count] = graphicsMeshInstance;
	window->graphicsMeshCount = count + 1;

	*graphicsMesh = graphicsMeshInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyGraphicsMesh(GraphicsMesh mesh)
{
	if (!mesh)
		return;

	assert(!mesh->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = mesh->base.window;
	GraphicsMesh* graphicsMeshes = window->graphicsMeshes;
	size_t graphicsMeshCount = window->graphicsMeshCount;

	for (size_t i = 0; i < graphicsMeshCount; i++)
	{
		if (mesh != graphicsMeshes[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkGraphicsMesh(mesh);
#else
			abort();
#endif
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			destroyGlGraphicsMesh(mesh);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < graphicsMeshCount; j++)
			graphicsMeshes[j - 1] = graphicsMeshes[j];

		window->graphicsMeshCount--;
		return;
	}

	abort();
}

Window getGraphicsMeshWindow(GraphicsMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.window;
}
IndexType getGraphicsMeshIndexType(GraphicsMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.indexType;
}

uint32_t getGraphicsMeshIndexCount(
	GraphicsMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.indexCount;
}
void setGraphicsMeshIndexCount(
	GraphicsMesh mesh,
	uint32_t indexCount)
{
	assert(mesh);
	assert(!mesh->base.window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	if (mesh->base.indexBuffer)
	{
		if (mesh->base.indexType == UINT16_INDEX_TYPE)
		{
			assert(indexCount * sizeof(uint16_t) +
				mesh->base.indexOffset * sizeof(uint16_t) <=
				mesh->base.indexBuffer->base.size);
		}
		else if (mesh->base.indexType == UINT32_INDEX_TYPE)
		{
			assert(indexCount * sizeof(uint32_t) +
				mesh->base.indexOffset * sizeof(uint32_t) <=
				mesh->base.indexBuffer->base.size);
		}
		else
		{
			abort();
		}
	}
#endif

	mesh->base.indexCount = indexCount;
}

uint32_t getGraphicsMeshIndexOffset(
	GraphicsMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.indexOffset;
}
void setGraphicsMeshIndexOffset(
	GraphicsMesh mesh,
	uint32_t indexOffset)
{
	assert(mesh);
	assert(!mesh->base.window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	if (mesh->base.indexBuffer)
	{
		if (mesh->base.indexType == UINT16_INDEX_TYPE)
		{
			assert(mesh->base.indexCount * sizeof(uint16_t) +
				indexOffset * sizeof(uint16_t) <=
				mesh->base.indexBuffer->base.size);
		}
		else if (mesh->base.indexType == UINT32_INDEX_TYPE)
		{
			assert(mesh->base.indexCount * sizeof(uint32_t) +
				indexOffset * sizeof(uint32_t) <=
				mesh->base.indexBuffer->base.size);
		}
		else
		{
			abort();
		}
	}
#endif

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		setVkGraphicsMeshIndexOffset(
			mesh,
			indexOffset);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		setGlGraphicsMeshIndexOffset(
			mesh,
			indexOffset);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

Buffer getGraphicsMeshVertexBuffer(
	GraphicsMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.vertexBuffer;
}
void setGraphicsMeshVertexBuffer(
	GraphicsMesh mesh,
	Buffer vertexBuffer)
{
	assert(mesh);
	assert(!mesh->base.window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	if (vertexBuffer)
	{
		assert(mesh->base.window == vertexBuffer->base.window);
		assert(vertexBuffer->base.type & VERTEX_BUFFER_TYPE);
	}
#endif

	mesh->base.vertexBuffer = vertexBuffer;
}

Buffer getGraphicsMeshIndexBuffer(
	GraphicsMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.indexBuffer;
}
void setGraphicsMeshIndexBuffer(
	GraphicsMesh mesh,
	IndexType indexType,
	uint32_t indexCount,
	uint32_t indexOffset,
	Buffer indexBuffer)
{
	assert(mesh);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(!mesh->base.window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	if (indexBuffer)
	{
		assert(mesh->base.window == indexBuffer->base.window);
		assert(indexBuffer->base.type & INDEX_BUFFER_TYPE);

		if (indexType == UINT16_INDEX_TYPE)
		{
			assert(indexCount * sizeof(uint16_t) +
				indexOffset * sizeof(uint16_t) <=
				indexBuffer->base.size);
		}
		else if (indexType == UINT32_INDEX_TYPE)
		{
			assert(indexCount * sizeof(uint32_t) +
				indexOffset * sizeof(uint32_t) <=
				indexBuffer->base.size);
		}
		else
		{
			abort();
		}
	}
#endif

	mesh->base.indexCount = indexCount;
	mesh->base.indexBuffer = indexBuffer;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		setVkGraphicsMeshIndexType(
			mesh,
			indexType);
		setVkGraphicsMeshIndexOffset(
			mesh,
			indexOffset);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		setGlGraphicsMeshIndexType(
			mesh,
			indexType);
		setGlGraphicsMeshIndexOffset(
			mesh,
			indexOffset);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

size_t drawGraphicsMesh(
	GraphicsPipeline pipeline,
	GraphicsMesh mesh)
{
	assert(mesh);
	assert(pipeline);
	assert(mesh->base.window->isRecording);
	assert(mesh->base.window == pipeline->base.window);
	assert(graphicsInitialized);

	if (!mesh->base.vertexBuffer ||
		!mesh->base.indexBuffer ||
		mesh->base.indexCount == 0)
	{
		return 0;
	}

	assert(!mesh->base.vertexBuffer->base.isMapped);
	assert(!mesh->base.indexBuffer->base.isMapped);

	Window window = mesh->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		drawVkGraphicsMesh(
			window->vkWindow->currenCommandBuffer,
			pipeline,
			mesh);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		drawGlGraphicsMesh(
			pipeline,
			mesh);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	return mesh->base.indexCount;
}

MpgxResult createComputePipeline(
	Window window,
	const char* name,
	OnComputePipelineBind onBind,
	OnComputePipelineDestroy onDestroy,
	void* handle,
	const void* createData,
	Shader shader,
	ComputePipeline* computePipeline)
{
	assert(window);
	assert(onDestroy);
	assert(handle);
	assert(shader);
	assert(computePipeline);
	assert(shader->base.type == COMPUTE_SHADER_TYPE);
	assert(shader->base.window == window);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	MpgxResult mpgxResult;
	ComputePipeline computePipelineInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		assert(createData);

		mpgxResult = createVkComputePipeline(
			window->vkWindow->device,
			createData,
			window,
			name,
			onBind,
			onDestroy,
			handle,
			shader,
			&computePipelineInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = window->computePipelineCount;

	if (count == window->computePipelineCapacity)
	{
		size_t capacity = window->computePipelineCapacity * 2;

		ComputePipeline* computePipelines = realloc(
			window->computePipelines,
			sizeof(ComputePipeline) * capacity);

		if (!computePipelines)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkComputePipeline(
					window->vkWindow->device,
					computePipelineInstance);
#else
				abort();
#endif
			}
			else
			{
				abort();
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		window->computePipelines = computePipelines;
		window->computePipelineCapacity = capacity;
	}

	window->computePipelines[count] = computePipelineInstance;
	window->computePipelineCount = count + 1;

	*computePipeline = computePipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyComputePipeline(ComputePipeline pipeline)
{
	if (!pipeline)
		return;

	assert(!pipeline->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = pipeline->base.window;
	size_t computePipelineCount = window->computePipelineCount;
	ComputePipeline* computePipelines = window->computePipelines;

	for (size_t i = 0; i < computePipelineCount; i++)
	{
		if (pipeline != computePipelines[i])
			continue;

		pipeline->base.onDestroy(
			window,
			pipeline->base.handle);

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->computeQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkComputePipeline(
				vkWindow->device,
				pipeline);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < computePipelineCount; j++)
			computePipelines[j - 1] = computePipelines[j];

		window->computePipelineCount--;
		return;
	}

	abort();
}

Window getComputePipelineWindow(ComputePipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.window;
}
const char* getComputePipelineName(ComputePipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
#ifndef NDEBUG
	return pipeline->base.name;
#else
	abort();
#endif
}
OnComputePipelineBind getComputePipelineOnBind(ComputePipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onBind;
}
OnComputePipelineDestroy getComputePipelineOnDestroy(ComputePipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onDestroy;
}
Shader getComputePipelineShader(ComputePipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.shader;
}
void* getComputePipelineHandle(ComputePipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.handle;
}

void bindComputePipeline(ComputePipeline pipeline)
{
	assert(pipeline);
	assert(pipeline->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = pipeline->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bindVkComputePipeline(
			window->vkWindow->currenCommandBuffer,
			pipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}
void dispatchComputePipeline(
	ComputePipeline pipeline,
	uint32_t groupCountX,
	uint32_t groupCountY,
	uint32_t groupCountZ)
{
	assert(pipeline);
	assert(groupCountX > 0 || groupCountY > 0 || groupCountZ > 0);
	assert(pipeline->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = pipeline->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		dispatchVkComputePipeline(
			window->vkWindow->currenCommandBuffer,
			groupCountX,
			groupCountY,
			groupCountZ);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

MpgxResult createRayTracingPipeline(
	Window window,
	const char* name,
	OnRayTracingPipelineBind onBind,
	OnRayTracingPipelineDestroy onDestroy,
	void* handle,
	const void* createData,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount,
	RayTracingPipeline* rayTracingPipeline)
{
	assert(window);
	assert(onDestroy);
	assert(handle);
	assert(createData);
	assert(generationShaders);
	assert(generationShaderCount > 0);
	assert(missShaders);
	assert(missShaderCount > 0);
	assert(closestHitShaders);
	assert(closestHitShaderCount > 0);
	assert(rayTracingPipeline);
	assert(window->useRayTracing);
	assert(!window->isRecording);
	assert(graphicsInitialized);

#ifndef NDEBUG
	for (size_t i = 0; i < generationShaderCount; i++)
	{
		Shader shader = generationShaders[i];
		assert(shader->base.type == RAY_GENERATION_SHADER_TYPE);
		assert(shader->base.window == window);
	}
	for (size_t i = 0; i < missShaderCount; i++)
	{
		Shader shader = missShaders[i];
		assert(shader->base.type == RAY_MISS_SHADER_TYPE);
		assert(shader->base.window == window);
	}
	for (size_t i = 0; i < closestHitShaderCount; i++)
	{
		Shader shader = closestHitShaders[i];
		assert(shader->base.type == RAY_CLOSEST_HIT_SHADER_TYPE);
		assert(shader->base.window == window);
	}
#endif

	RayTracing rayTracing = window->rayTracing;

	MpgxResult mpgxResult;
	RayTracingPipeline rayTracingPipelineInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		mpgxResult = createVkRayTracingPipeline(
			vkWindow->device,
			vkWindow->allocator,
			createData,
			rayTracing,
			name,
			window,
			onBind,
			onDestroy,
			handle,
			generationShaders,
			generationShaderCount,
			missShaders,
			missShaderCount,
			closestHitShaders,
			closestHitShaderCount,
			&rayTracingPipelineInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = rayTracing->base.pipelineCount;

	if (count == rayTracing->base.pipelineCapacity)
	{
		size_t capacity = rayTracing->base.pipelineCapacity * 2;

		RayTracingPipeline* pipelines = realloc(
			rayTracing->base.pipelines,
			sizeof(RayTracingPipeline) * capacity);

		if (!pipelines)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkRayTracingPipeline(
					vkWindow->device,
					vkWindow->allocator,
					rayTracingPipelineInstance);
#else
				abort();
#endif
			}
			else
			{
				abort();
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		rayTracing->base.pipelines = pipelines;
		rayTracing->base.pipelineCapacity = capacity;
	}

	rayTracing->base.pipelines[count] = rayTracingPipelineInstance;
	rayTracing->base.pipelineCount = count + 1;

	*rayTracingPipeline = rayTracingPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyRayTracingPipeline(RayTracingPipeline pipeline)
{
	if (!pipeline)
		return;

	assert(!pipeline->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = pipeline->base.window;
	RayTracing rayTracing = window->rayTracing;
	size_t pipelineCount = rayTracing->base.pipelineCount;
	RayTracingPipeline* pipelines = rayTracing->base.pipelines;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		if (pipeline != pipelines[i])
			continue;

		pipeline->base.onDestroy(
			window,
			pipeline->base.handle);

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkRayTracingPipeline(
				vkWindow->device,
				vkWindow->allocator,
				pipeline);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < pipelineCount; j++)
			pipelines[j - 1] = pipelines[j];

		rayTracing->base.pipelineCount--;
		return;
	}

	abort();
}

Window getRayTracingPipelineWindow(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.window;
}
const char* getRayTracingPipelineName(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
#ifndef NDEBUG
	return pipeline->base.name;
#else
	abort();
#endif
}
OnRayTracingPipelineBind getRayTracingPipelineOnBind(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onBind;
}
OnRayTracingPipelineDestroy getRayTracingPipelineOnDestroy(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.onDestroy;
}
void* getRayTracingPipelineHandle(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.handle;
}
Shader* getRayTracingPipelineGenerationShaders(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.generationShaders;
}
size_t getRayTracingPipelineGenerationShaderCount(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.generationShaderCount;
}
Shader* getRayTracingPipelineMissShaders(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.missShaders;
}
size_t getRayTracingPipelineMissShaderCount(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.missShaderCount;
}
Shader* getRayTracingPipelineClosestHitShaders(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.closestHitShaders;
}
size_t getRayTracingPipelineClosestHitShaderCount(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(graphicsInitialized);
	return pipeline->base.closestHitShaderCount;
}

void bindRayTracingPipeline(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(pipeline->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = pipeline->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bindVkRayTracingPipeline(
			window->vkWindow->currenCommandBuffer,
			pipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

void traceRayTracingPipeline(RayTracingPipeline pipeline)
{
	assert(pipeline);
	assert(pipeline->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = pipeline->base.window;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		traceVkPipelineRays(
			window->vkWindow->currenCommandBuffer,
			window->rayTracing,
			pipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

MpgxResult createRayTracingMesh(
	Window window,
	size_t vertexStride,
	IndexType indexType,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	RayTracingMesh* rayTracingMesh)
{
	assert(window);
	assert(vertexStride > 0);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(vertexBuffer);
	assert(indexBuffer);
	assert(rayTracingMesh);
	assert(vertexBuffer->base.type & VERTEX_BUFFER_TYPE);
	assert(indexBuffer->base.type & INDEX_BUFFER_TYPE);
	assert(vertexBuffer->base.window == window);
	assert(indexBuffer->base.window == window);
	assert(!vertexBuffer->base.isMapped);
	assert(!indexBuffer->base.isMapped);
	assert(window->useRayTracing);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	RayTracing rayTracing = window->rayTracing;

	MpgxResult mpgxResult;
	RayTracingMesh rayTracingMeshInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		mpgxResult = createVkRayTracingMesh(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->transferQueue,
			vkWindow->transferCommandBuffer,
			vkWindow->transferFence,
			rayTracing,
			window,
			vertexStride,
			indexType,
			vertexBuffer,
			indexBuffer,
			&rayTracingMeshInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = rayTracing->base.meshCount;

	if (count == rayTracing->base.meshCapacity)
	{
		size_t capacity = rayTracing->base.meshCapacity * 2;

		RayTracingMesh* meshes = realloc(
			rayTracing->base.meshes,
			sizeof(RayTracingMesh) * capacity);

		if (!meshes)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkRayTracingMesh(
					vkWindow->device,
					vkWindow->allocator,
					window->rayTracing,
					rayTracingMeshInstance);
#else
				abort();
#endif
			}
			else
			{
				abort();
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		rayTracing->base.meshes = meshes;
		rayTracing->base.meshCapacity = capacity;
	}

	rayTracing->base.meshes[count] = rayTracingMeshInstance;
	rayTracing->base.meshCount = count + 1;

	*rayTracingMesh = rayTracingMeshInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyRayTracingMesh(RayTracingMesh mesh)
{
	if (!mesh)
		return;

	assert(!mesh->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = mesh->base.window;
	RayTracing rayTracing = window->rayTracing;
	RayTracingMesh* meshes = rayTracing->base.meshes;
	size_t meshCount = rayTracing->base.meshCount;

	for (size_t i = 0; i < meshCount; i++)
	{
		if (mesh != meshes[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkRayTracingMesh(
				vkWindow->device,
				vkWindow->allocator,
				window->rayTracing,
				mesh);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < meshCount; j++)
			meshes[j - 1] = meshes[j];

		rayTracing->base.meshCount--;
		return;
	}

	abort();
}

Window getRayTracingMeshWindow(RayTracingMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.window;
}
size_t getRayTracingMeshVertexStride(RayTracingMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.vertexStride;
}
IndexType getRayTracingMeshIndexType(RayTracingMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.indexType;
}
Buffer getRayTracingMeshVertexBuffer(RayTracingMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.vertexBuffer;
}
Buffer getRayTracingMeshIndexBuffer(RayTracingMesh mesh)
{
	assert(mesh);
	assert(graphicsInitialized);
	return mesh->base.indexBuffer;
}

MpgxResult createRayTracingScene(
	Window window,
	RayTracingMesh* meshes,
	size_t meshCount,
	RayTracingScene* rayTracingScene)
{
	assert(window);
	assert(meshes);
	assert(meshCount > 0);
	assert(rayTracingScene);
	assert(window->useRayTracing);
	assert(!window->isRecording);
	assert(graphicsInitialized);

	RayTracing rayTracing = window->rayTracing;

	MpgxResult mpgxResult;
	RayTracingScene rayTracingSceneInstance;

	if (graphicsAPI == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		mpgxResult = createVkRayTracingScene(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->transferQueue,
			vkWindow->transferCommandBuffer,
			vkWindow->transferFence,
			rayTracing,
			window,
			meshes,
			meshCount,
			&rayTracingSceneInstance);
#else
		abort();
#endif
	}
	else if (graphicsAPI == OPENGL_GRAPHICS_API ||
		graphicsAPI == OPENGL_ES_GRAPHICS_API)
	{
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	}
	else
	{
		abort();
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	size_t count = rayTracing->base.sceneCount;

	if (count == rayTracing->base.sceneCapacity)
	{
		size_t capacity = rayTracing->base.sceneCapacity * 2;

		RayTracingScene* scenes = realloc(
			rayTracing->base.scenes,
			sizeof(RayTracingScene) * capacity);

		if (!scenes)
		{
			if (graphicsAPI == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkRayTracingScene(
					vkWindow->device,
					vkWindow->allocator,
					window->rayTracing,
					rayTracingSceneInstance);
#else
				abort();
#endif
			}
			else
			{
				abort();
			}

			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		rayTracing->base.scenes = scenes;
		rayTracing->base.sceneCapacity = capacity;
	}

	rayTracing->base.scenes[count] = rayTracingSceneInstance;
	rayTracing->base.sceneCount = count + 1;

	*rayTracingScene = rayTracingSceneInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyRayTracingScene(RayTracingScene scene)
{
	if (!scene)
		return;

	assert(!scene->base.window->isRecording);
	assert(graphicsInitialized);

	Window window = scene->base.window;
	RayTracing rayTracing = window->rayTracing;
	RayTracingScene* scenes = rayTracing->base.scenes;
	size_t sceneCount = rayTracing->base.sceneCount;

	for (size_t i = 0; i < sceneCount; i++)
	{
		if (scene != scenes[i])
			continue;

		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkRayTracingScene(
				vkWindow->device,
				vkWindow->allocator,
				window->rayTracing,
				scene);
#else
			abort();
#endif
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < sceneCount; j++)
			scenes[j - 1] = scenes[j];

		rayTracing->base.sceneCount--;
		return;
	}

	abort();
}

Window getRayTracingSceneWindow(RayTracingScene scene)
{
	assert(scene);
	assert(graphicsInitialized);
	return scene->base.window;
}
RayTracingMesh* getRayTracingSceneMeshes(RayTracingScene scene)
{
	assert(scene);
	assert(graphicsInitialized);
	return scene->base.meshes;
}
size_t getRayTracingSceneMeshCount(RayTracingScene scene)
{
	assert(scene);
	assert(graphicsInitialized);
	return scene->base.meshCount;
}
