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

#include "mpgx/window.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/mesh.h"
#include "mpgx/_source/sampler.h"
#include "mpgx/_source/framebuffer.h"
#include "mpgx/_source/ray_tracing.h"

#include "mpio/file.h"
#include "cmmt/common.h"
#include "mpmt/thread.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>

// TODO: add VMA defragmentation

struct ImageData_T
{
	uint8_t* pixels;
	Vec2U size;
	uint8_t channelCount;
};

struct Window_T
{
	GraphicsAPI api;
	bool useStencilBuffer;
	bool useRayTracing;
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
	Mesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
	double targetFPS;
	double updateTime;
	double deltaTime;
	bool isRecording;
	Framebuffer renderFramebuffer;
#if MPGX_SUPPORT_VULKAN
	VkWindow vkWindow;
#endif
};

static bool graphicsInitialized = false;
static FT_Library ftLibrary = NULL;
static Window currentWindow = NULL;

#if MPGX_SUPPORT_VULKAN
static VkInstance vkInstance = NULL;
static VkDebugUtilsMessengerEXT vkDebugUtilsMessenger = NULL;
#endif

static void glfwErrorCallback(
	int error,
	const char* description)
{
	fprintf(stdout,
		"GLFW ERROR: %d, %s\n",
		error,
		description);
}

inline static void terminateFreeTypeLibrary(
	FT_Library freeTypeLibrary)
{
	if (FT_Done_FreeType(freeTypeLibrary) != 0)
		abort();
}
MpgxResult initializeGraphics(
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch)
{
	assert(appName != NULL);

	if (graphicsInitialized == true)
		return GRAPHICS_IS_ALREADY_INITIALIZED_MPGX_RESULT;

	if(glfwInit() == GLFW_FALSE)
		return FAILED_TO_INITIALIZE_GLFW_MPGX_RESULT;

	glfwSetErrorCallback(glfwErrorCallback);

	if (FT_Init_FreeType(&ftLibrary) != 0)
	{
		glfwTerminate();
		return FAILED_TO_INITIALIZE_FREETYPE_MPGX_RESULT;
	}

#if MPGX_SUPPORT_VULKAN
	uint32_t glfwExtensionCount;

	const char** glfwExtensions =
		glfwGetRequiredInstanceExtensions(
		&glfwExtensionCount);

	if (glfwExtensionCount == 0 || glfwExtensions == NULL)
	{
		terminateFreeTypeLibrary(ftLibrary);
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

	bool result = checkVkInstanceLayers(
		targetLayers,
		isLayerSupported,
		targetLayerCount);

	if (result == false)
	{
		terminateFreeTypeLibrary(ftLibrary);
		glfwTerminate();
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

#ifndef NDEBUG
	if (isLayerSupported[validationLayerIndex] == true)
		layers[layerCount++] = targetLayers[validationLayerIndex];
#endif

	const char* targetExtensions[1];
	bool isExtensionSupported[1];
	uint32_t extensionCount = glfwExtensionCount;
	uint32_t targetExtensionCount = 0;

	const char** extensions = malloc(
		(1 + glfwExtensionCount) * sizeof(const char*));

	if (extensions == NULL)
	{
		terminateFreeTypeLibrary(ftLibrary);
		glfwTerminate();
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < glfwExtensionCount; i++)
		extensions[i] = glfwExtensions[i];

#ifndef NDEBUG
	extensions[extensionCount++] =
		targetExtensions[targetExtensionCount] =
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	uint32_t debugUtilsExtIndex = targetExtensionCount++;
#endif

	result = checkVkInstanceExtensions(
		targetExtensions,
		isExtensionSupported,
		targetExtensionCount);

	if (result == false)
	{
		free((void*)extensions);
		terminateFreeTypeLibrary(ftLibrary);
		glfwTerminate();
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

#ifndef NDEBUG
	if (isExtensionSupported[debugUtilsExtIndex] == false)
	{
		free((void*)extensions);
		terminateFreeTypeLibrary(ftLibrary);
		glfwTerminate();
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}
#endif

	vkInstance = createVkInstance(
		appName,
		appVersionMajor,
		appVersionMinor,
		appVersionPatch,
		layers,
		layerCount,
		extensions,
		extensionCount);

	free((void*)extensions);

	if (vkInstance == NULL)
	{
		terminateFreeTypeLibrary(ftLibrary);
		glfwTerminate();
		return FAILED_TO_INITIALIZE_VULKAN_MPGX_RESULT;
	}

#ifndef NDEBUG
	vkDebugUtilsMessenger =
		createVkDebugUtilsMessenger(vkInstance);

	if (vkDebugUtilsMessenger == NULL)
	{
		vkDestroyInstance(
			vkInstance,
			NULL);
		terminateFreeTypeLibrary(ftLibrary);
		glfwTerminate();
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}
#endif
#endif

	graphicsInitialized = true;
	return true;
}
void terminateGraphics()
{
	if (graphicsInitialized == true)
		return;

#if MPGX_SUPPORT_VULKAN

#ifndef NDEBUG
	destroyVkDebugUtilsMessenger(
		vkInstance,
		vkDebugUtilsMessenger);
#endif

	vkDestroyInstance(
		vkInstance,
		NULL);
#endif

	terminateFreeTypeLibrary(ftLibrary);
	glfwTerminate();

	graphicsInitialized = false;
}
bool isGraphicsInitialized()
{
	return graphicsInitialized;
}

void* getFtLibrary()
{
	return ftLibrary;
}

static void onWindowChar(
	GLFWwindow* handle,
	unsigned int codepoint)
{
	Window window = glfwGetWindowUserPointer(handle);

	size_t length = window->inputLength;

	if (length == window->inputCapacity)
	{
		size_t capacity = window->inputCapacity * 2;

		uint32_t* inputBuffer = realloc(
			window->inputBuffer,
			capacity * sizeof(uint32_t));

		if (inputBuffer == NULL)
			abort();

		window->inputBuffer = inputBuffer;
		window->inputCapacity = capacity;
	}

	window->inputBuffer[length] = codepoint;
	window->inputLength = length + 1;
}

void destroyWindow(Window window)
{
	if (window == NULL)
		return;

	assert(window->bufferCount == 0);
	assert(window->imageCount == 0);
	assert(window->samplerCount == 0);
	assert(window->framebufferCount == 0);
	assert(window->shaderCount == 0);
	assert(window->meshCount == 0);

	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		if (vkWindow != NULL)
		{
			VkDevice device = vkWindow->device;
			VkResult result = vkDeviceWaitIdle(device);

			if (result != VK_SUCCESS)
				abort();

			destroyVkFramebuffer(
				device,
				window->framebuffer,
				false);
			destroyVkRayTracing(
				window->rayTracing);
			destroyVkWindow(
				vkInstance,
				vkWindow);
		}
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		destroyGlFramebuffer(
			window->framebuffer,
			false);
	}
	else
	{
		abort();
	}

	free(window->meshes);
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
MpgxResult createWindow(
	GraphicsAPI api,
	Vec2U size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool useStencilBuffer,
	bool useRayTracing,
	bool isVisible,
	Window* window)
{
	assert(api < GRAPHICS_API_COUNT);
	assert(size.x != 0);
	assert(size.y != 0);
	assert(title != NULL);
	assert(onUpdate != NULL);
	assert(window != NULL);

	if (graphicsInitialized == false)
		return GRAPHICS_IS_NOT_INITIALIZED_MPGX_RESULT;

	glfwDefaultWindowHints();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_NO_API);
#else
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
#endif
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
		if (useRayTracing == true)
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

		if (useStencilBuffer == true)
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
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		if (useRayTracing == true)
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

		if (useStencilBuffer == true)
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
	}
	else
	{
		abort();
	}

	glfwWindowHint(GLFW_VISIBLE,
		isVisible ? GLFW_TRUE : GLFW_FALSE);

	Window windowInstance = calloc(1,
		sizeof(Window_T));

	if (windowInstance == NULL)
		return FAILED_TO_ALLOCATE_MPGX_RESULT;

	windowInstance->api = api;
	windowInstance->useStencilBuffer = useStencilBuffer;
	windowInstance->useRayTracing = useRayTracing;
	windowInstance->onUpdate = onUpdate;
	windowInstance->updateArgument = updateArgument;

	GLFWwindow* handle = glfwCreateWindow(
		(int)size.x,
		(int)size.y,
		title,
		NULL,
		NULL);

	if (handle == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->handle = handle;

	glfwSetWindowSizeLimits(
		handle,
		2,
		2,
		GLFW_DONT_CARE,
		GLFW_DONT_CARE);

	if (glfwRawMouseMotionSupported() == GLFW_TRUE)
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

	if (ibeamCursor == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->cursorType = DEFAULT_CURSOR_TYPE;
	windowInstance->ibeamCursor = ibeamCursor;

	GLFWcursor* crosshairCursor = glfwCreateStandardCursor(
		GLFW_CROSSHAIR_CURSOR);

	if (crosshairCursor == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->crosshairCursor = crosshairCursor;

	GLFWcursor* handCursor = glfwCreateStandardCursor(
		GLFW_HAND_CURSOR);

	if (handCursor == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->handCursor = handCursor;

	GLFWcursor* hresizeCursor = glfwCreateStandardCursor(
		GLFW_HRESIZE_CURSOR);

	if (hresizeCursor == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->hresizeCursor = hresizeCursor;

	GLFWcursor* vresizeCursor = glfwCreateStandardCursor(
		GLFW_VRESIZE_CURSOR);

	if (vresizeCursor == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->vresizeCursor = vresizeCursor;

	uint32_t* inputBuffer = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(uint32_t));

	if (inputBuffer == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->inputBuffer = inputBuffer;
	windowInstance->inputCapacity = MPGX_DEFAULT_CAPACITY;
	windowInstance->inputLength = 0;

#if MPGX_SUPPORT_VULKAN
	VkWindow vkWindow = NULL;
#endif

	int width, height;

	glfwGetFramebufferSize(
		handle,
		&width,
		&height);

	Vec2U framebufferSize =
		vec2U(width, height);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		MpgxResult mpgxResult = createVkWindow(
			vkInstance,
			handle,
			useStencilBuffer,
			useRayTracing,
			framebufferSize,
			&vkWindow);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyWindow(windowInstance);
			return mpgxResult;
		}

		windowInstance->vkWindow = vkWindow;

		if (useRayTracing == true)
		{
			RayTracing rayTracing = createVkRayTracing(
				vkInstance,
				vkWindow->physicalDevice);

			if (rayTracing == NULL)
			{
				destroyWindow(windowInstance);
				return FAILED_TO_ALLOCATE_MPGX_RESULT;
			}

			windowInstance->rayTracing = rayTracing;
		}
		else
		{
			windowInstance->rayTracing = NULL;
		}

		VkSwapchain swapchain = vkWindow->swapchain;
		VkSwapchainBuffer firstBuffer = swapchain->buffers[0];

		Framebuffer framebuffer = createVkDefaultFramebuffer(
			vkWindow->device,
			swapchain->renderPass,
			firstBuffer.framebuffer,
			windowInstance,
			framebufferSize);

		if (framebuffer == NULL)
		{
			destroyWindow(windowInstance);
			return FAILED_TO_ALLOCATE_MPGX_RESULT;
		}

		windowInstance->framebuffer = framebuffer;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		glfwMakeContextCurrent(handle);

		if (gladLoadGL() == 0)
		{
			destroyWindow(windowInstance);
			return FAILED_TO_INITIALIZE_OPENGL_MPGX_RESULT;
		}

		glEnable(GL_FRAMEBUFFER_SRGB);

		Framebuffer framebuffer = createGlDefaultFramebuffer(
			windowInstance,
			framebufferSize);

		if (framebuffer == NULL)
		{
			destroyWindow(windowInstance);
			return FAILED_TO_ALLOCATE_MPGX_RESULT;
		}

		windowInstance->framebuffer = framebuffer;
	}

	Buffer* buffers = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Buffer));

	if (buffers == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->buffers = buffers;
	windowInstance->bufferCapacity = MPGX_DEFAULT_CAPACITY;
	windowInstance->bufferCount = 0;

	Image* images = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Image));

	if (images == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->images = images;
	windowInstance->imageCapacity = MPGX_DEFAULT_CAPACITY;
	windowInstance->imageCount = 0;

	Sampler* samplers = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Sampler));

	if (samplers == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->samplers = samplers;
	windowInstance->samplerCapacity = MPGX_DEFAULT_CAPACITY;
	windowInstance->samplerCount = 0;

	Shader* shaders = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Shader));

	if (shaders == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->shaders = shaders;
	windowInstance->shaderCapacity = MPGX_DEFAULT_CAPACITY;
	windowInstance->shaderCount = 0;

	Framebuffer* framebuffers = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Framebuffer));

	if (framebuffers == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->framebuffers = framebuffers;
	windowInstance->framebufferCapacity = MPGX_DEFAULT_CAPACITY;
	windowInstance->framebufferCount = 0;

	Mesh* meshes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Mesh));

	if (meshes == NULL)
	{
		destroyWindow(windowInstance);
		return FAILED_TO_ALLOCATE_MPGX_RESULT;
	}

	windowInstance->meshes = meshes;
	windowInstance->meshCapacity = MPGX_DEFAULT_CAPACITY;
	windowInstance->meshCount = 0;

	windowInstance->targetFPS = 60.0;
	windowInstance->updateTime = 0.0;
	windowInstance->deltaTime = 0.0;
	windowInstance->isRecording = false;
	windowInstance->renderFramebuffer = NULL;

	currentWindow = windowInstance;
	*window = windowInstance;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createAnyWindow(
	Vec2U size,
	const char* title,
	OnWindowUpdate updateFunction,
	void* updateArgument,
	bool useStencilBuffer,
	bool useRayTracing,
	bool visible,
	Window* window)
{
	MpgxResult mpgxResult = createWindow(
		VULKAN_GRAPHICS_API,
		size,
		title,
		updateFunction,
		updateArgument,
		useStencilBuffer,
		useRayTracing,
		visible,
		window);

	if (mpgxResult == SUCCESS_MPGX_RESULT)
		return SUCCESS_MPGX_RESULT;

	mpgxResult = createWindow(
		OPENGL_GRAPHICS_API,
		size,
		title,
		updateFunction,
		updateArgument,
		useStencilBuffer,
		useRayTracing,
		visible,
		window);

	if (mpgxResult == SUCCESS_MPGX_RESULT)
		return SUCCESS_MPGX_RESULT;

	return createWindow(
		OPENGL_ES_GRAPHICS_API,
		size,
		title,
		updateFunction,
		updateArgument,
		useStencilBuffer,
		useRayTracing,
		visible,
		window);
}

GraphicsAPI getWindowGraphicsAPI(Window window)
{
	assert(window != NULL);
	return window->api;
}
bool isWindowUseStencilBuffer(Window window)
{
	assert(window != NULL);
	return window->useStencilBuffer;
}
bool isWindowUseRayTracing(Window window)
{
	assert(window != NULL);
	return window->useRayTracing;
}
OnWindowUpdate getWindowOnUpdate(Window window)
{
	assert(window != NULL);
	return window->onUpdate;
}
void* getWindowUpdateArgument(Window window)
{
	assert(window != NULL);
	return window->updateArgument;
}
const uint32_t* getWindowInputBuffer(Window window)
{
	assert(window != NULL);
	return window->inputBuffer;
}
size_t getWindowInputLength(Window window)
{
	assert(window != NULL);
	return window->inputLength;
}
Framebuffer getWindowFramebuffer(Window window)
{
	assert(window != NULL);
	return window->framebuffer;
}
double getWindowUpdateTime(Window window)
{
	assert(window != NULL);
	return window->updateTime;
}
double getWindowDeltaTime(Window window)
{
	assert(window != NULL);
	return window->deltaTime;
}
Vec2F getWindowContentScale(Window window)
{
	assert(window != NULL);

	Vec2F scale;

	glfwGetWindowContentScale(
		window->handle,
		&scale.x,
		&scale.y);

	return scale;
}

inline static const char* getGlWindowGpuName()
{
	const char* name = (const char*)
		glGetString(GL_RENDERER);
	assertOpenGL();
	return name;
}
const char* getWindowGpuName(Window window)
{
	assert(window != NULL);

	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return getVkWindowGpuName(
			window->vkWindow->physicalDevice);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return getGlWindowGpuName();
	}
	else
	{
		abort();
	}
}

void* getVkWindow(Window window)
{
	assert(window != NULL);
	assert(window->api == VULKAN_GRAPHICS_API);

#if MPGX_SUPPORT_VULKAN
	return window->vkWindow;
#else
	abort();
#endif
}
bool isVkGpuIntegrated(Window window)
{
	assert(window != NULL);
	assert(window->api == VULKAN_GRAPHICS_API);

#if MPGX_SUPPORT_VULKAN
	return window->vkWindow->isGpuIntegrated;
#else
	abort();
#endif
}

double getWindowTargetFPS(
	Window window)
{
	assert(window != NULL);
	return window->targetFPS;
}
void setWindowTargetFPS(
	Window window,
	double fps)
{
	assert(window != NULL);
	assert(fps >= 0.0);
	window->targetFPS = fps;
}

bool getWindowKeyboardKey(
	Window window,
	KeyboardKey key)
{
	assert(window != NULL);

	return glfwGetKey(
		window->handle,
		key) == GLFW_PRESS;
}
bool getWindowMouseButton(
	Window window,
	MouseButton button)
{
	assert(window != NULL);

	return glfwGetMouseButton(
		window->handle,
		button) == GLFW_PRESS;
}

const char* getWindowClipboard(
	Window window)
{
	assert(window != NULL);
	return glfwGetClipboardString(window->handle);
}
void setWindowClipboard(
	Window window,
	const char* clipboard)
{
	assert(window != NULL);
	assert(clipboard != NULL);

	glfwSetClipboardString(
		window->handle,
		clipboard);
}

Vec2U getWindowSize(
	Window window)
{
	assert(window != NULL);

	int width, height;

	glfwGetWindowSize(
		window->handle,
		&width,
		&height);

	return vec2U(width, height);
}
void setWindowSize(
	Window window,
	Vec2U size)
{
	assert(window != NULL);

	glfwSetWindowSize(
		window->handle,
		(int)size.x,
		(int)size.y);
}

Vec2I getWindowPosition(
	Window window)
{
	assert(window != NULL);

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
	assert(window != NULL);

	glfwSetWindowPos(
		window->handle,
		position.x,
		position.y);
}

Vec2F getWindowCursorPosition(
	Window window)
{
	assert(window != NULL);

	double x, y;

	glfwGetCursorPos(
		window->handle,
		&x,
		&y);

	return vec2F((float)x, (float)y);
}
void setWindowCursorPosition(
	Window window,
	Vec2F position)
{
	assert(window != NULL);

	glfwSetCursorPos(
		window->handle,
		(double)position.x,
		(double)position.y);
}

CursorMode getWindowCursorMode(
	Window window)
{
	assert(window != NULL);

	return glfwGetInputMode(
		window->handle,
		GLFW_CURSOR);
}
void setWindowCursorMode(
	Window window,
	CursorMode cursorMode)
{
	assert(window != NULL);

	glfwSetInputMode(
		window->handle,
		GLFW_CURSOR,
		cursorMode);
}

CursorType getWindowCursorType(
	Window window)
{
	assert(window != NULL);
	return window->cursorType;
}
void setWindowCursorType(
	Window window,
	CursorType cursorType)
{
	assert(window != NULL);
	assert(cursorType < CURSOR_TYPE_COUNT);

	switch (cursorType)
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
	assert(window != NULL);
	assert(title != NULL);
	glfwSetWindowTitle(window->handle, title);
}

bool isWindowFocused(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_FOCUSED) == GLFW_TRUE;
}
bool isWindowIconified(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_ICONIFIED) == GLFW_TRUE;
}
bool isWindowMaximized(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_MAXIMIZED) == GLFW_TRUE;
}
bool isWindowVisible(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_VISIBLE) == GLFW_TRUE;
}
bool isWindowHovered(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->handle,
		GLFW_HOVERED) == GLFW_TRUE;
}

void iconifyWindow(Window window)
{
	assert(window != NULL);
	glfwIconifyWindow(window->handle);
}
void maximizeWindow(Window window)
{
	assert(window != NULL);
	glfwMaximizeWindow(window->handle);
}
void restoreWindow(Window window)
{
	assert(window != NULL);
	glfwRestoreWindow(window->handle);
}
void showWindow(Window window)
{
	assert(window != NULL);
	glfwShowWindow(window->handle);
}
void hideWindow(Window window)
{
	assert(window != NULL);
	glfwHideWindow(window->handle);
}
void focusWindow(Window window)
{
	assert(window != NULL);
	glfwFocusWindow(window->handle);
}
void requestWindowAttention(Window window)
{
	assert(window != NULL);
	glfwRequestWindowAttention(window->handle);
}

void makeWindowContextCurrent(Window window)
{
	assert(window != NULL);
	assert(window->api == OPENGL_GRAPHICS_API ||
		window->api == OPENGL_ES_GRAPHICS_API);
	assert(window->isRecording == false);

	if (window != currentWindow)
	{
		glfwMakeContextCurrent(window->handle);
		currentWindow = window;
	}
}
void updateWindow(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == false);

	GLFWwindow* handle = window->handle;
	OnWindowUpdate onUpdate = window->onUpdate;
	void* updateArgument = window->updateArgument;

	while (glfwWindowShouldClose(handle) == GLFW_FALSE)
	{
		window->inputLength = 0;
		glfwPollEvents();

		double startTime = getCurrentClock();
		window->deltaTime = startTime - window->updateTime;
		window->updateTime = startTime;

		onUpdate(updateArgument);

		if (window->targetFPS > 0.0)
		{
			double frameTime = 1.0 / window->targetFPS;

			double delayTime = frameTime -
				(getCurrentClock() - startTime) - 0.001;

			if (delayTime > 0.0)
				sleepThread(delayTime);
		}
	}
}

#if MPGX_SUPPORT_VULKAN
static bool onVkResize(
	VkDevice device,
	VkSwapchain swapchain,
	Framebuffer framebuffer,
	Vec2U newSize)
{
	VkSwapchainBuffer firstBuffer = swapchain->buffers[0];
	framebuffer->vk.size = newSize;
	framebuffer->vk.renderPass = swapchain->renderPass;
	framebuffer->vk.handle = firstBuffer.framebuffer;

	Pipeline* pipelines = framebuffer->vk.pipelines;
	size_t pipelineCount = framebuffer->vk.pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];
		VkPipelineCreateInfo createInfo;

		bool result = pipeline->vk.onResize(
			pipeline,
			newSize,
			&createInfo);
		result &= recreateVkPipelineHandle(
			device,
			framebuffer->vk.renderPass,
			pipeline,
			framebuffer->vk.colorAttachmentCount,
			&createInfo);

		if (result == false)
			return false;
	}

	return true;
}
#endif

bool beginWindowRecord(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == false);

	int width, height;

	glfwGetFramebufferSize(
		window->handle,
		&width,
		&height);

	if (width <= 0 || height <= 0)
		return false;

	Vec2U newSize = vec2U(width, height);
	Framebuffer framebuffer = window->framebuffer;

	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VmaAllocator allocator = vkWindow->allocator;

		if (vkWindow->stagingBuffer != NULL)
		{
			vmaDestroyBuffer(
				vkWindow->allocator,
				vkWindow->stagingBuffer,
				vkWindow->stagingAllocation);
			vkWindow->stagingBuffer = NULL;
			vkWindow->stagingAllocation = NULL;
			vkWindow->stagingSize = 0;
		}

		VkSwapchain swapchain = vkWindow->swapchain;
		VkDevice device = vkWindow->device;

		if (!compVec2U(newSize, framebuffer->gl.size))
		{
			bool result = resizeVkSwapchain(
				vkWindow->surface,
				vkWindow->physicalDevice,
				vkWindow->graphicsQueueFamilyIndex,
				vkWindow->presentQueueFamilyIndex,
				device,
				allocator,
				vkWindow->graphicsCommandPool,
				vkWindow->presentCommandPool,
				swapchain,
				window->useStencilBuffer,
				newSize);

			if (result == false)
				return false;

			vkWindow->frameIndex = 0;

			result = onVkResize(
				device,
				swapchain,
				framebuffer,
				newSize);

			if (result == false)
				return false;
		}

		uint32_t frameIndex = vkWindow->frameIndex;
		VkFence fence = vkWindow->fences[frameIndex];

		VkResult vkResult = vkWaitForFences(
			device,
			1,
			&fence,
			VK_TRUE,
			UINT64_MAX);

		if (vkResult != VK_SUCCESS &&
			vkResult != VK_TIMEOUT)
		{
			return false;
		}

		vkResult = vkResetFences(
			device,
			1,
			&fence);

		if (vkResult != VK_SUCCESS)
			return false;

		VkSemaphore* imageAcquiredSemaphores =
			vkWindow->imageAcquiredSemaphores;
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
				bool result = resizeVkSwapchain(
					vkWindow->surface,
					vkWindow->physicalDevice,
					vkWindow->graphicsQueueFamilyIndex,
					vkWindow->presentQueueFamilyIndex,
					device,
					allocator,
					vkWindow->graphicsCommandPool,
					vkWindow->presentCommandPool,
					swapchain,
					window->useStencilBuffer,
					newSize);

				if (result == false)
					return false;

				vkWindow->frameIndex = 0;

				result = onVkResize(
					device,
					swapchain,
					framebuffer,
					newSize);

				if (result == false)
					return false;
			}
			else if (vkResult != VK_SUCCESS &&
				vkResult != VK_SUBOPTIMAL_KHR &&
				vkResult == VK_ERROR_SURFACE_LOST_KHR)
			{
				return false;
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
			return false;

		vkWindow->bufferIndex = bufferIndex;
		vkWindow->currenCommandBuffer = graphicsCommandBuffer;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		if (!compVec2U(newSize, framebuffer->gl.size))
		{
			framebuffer->gl.size = newSize;

			Pipeline* pipelines = framebuffer->gl.pipelines;
			size_t pipelineCount = framebuffer->gl.pipelineCount;

			for (size_t i = 0; i < pipelineCount; i++)
			{
				Pipeline pipeline = pipelines[i];

				pipeline->gl.onResize(
					pipeline,
					newSize,
					NULL);
			}
		}
	}
	else
	{
		abort();
	}

	window->isRecording = true;
	return true;
}

void endWindowRecord(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == true);
	assert(window->renderFramebuffer == NULL);

	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
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

			Vec2U newSize = vec2U(width, height);
			Framebuffer framebuffer = window->framebuffer;
			VkDevice device = vkWindow->device;

			bool result = resizeVkSwapchain(
				vkWindow->surface,
				vkWindow->physicalDevice,
				graphicsQueueFamilyIndex,
				presentQueueFamilyIndex,
				device,
				vkWindow->allocator,
				vkWindow->graphicsCommandPool,
				vkWindow->presentCommandPool,
				swapchain,
				window->useStencilBuffer,
				newSize);

			if (result == false)
				abort();

			vkWindow->frameIndex = 0;

			result = onVkResize(
				device,
				swapchain,
				framebuffer,
				newSize);

			if (result == false)
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
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		glfwSwapBuffers(window->handle);
	}
	else
	{
		abort();
	}

	window->isRecording = false;
}

Buffer createBuffer(
	Window window,
	BufferType type,
	const void* data,
	size_t size,
	bool isConstant)
{
	assert(window != NULL);
	assert(type < BUFFER_TYPE_COUNT);
	assert(size != 0);
	assert(window->isRecording == false);

	GraphicsAPI api = window->api;

	Buffer buffer;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		buffer = createVkBuffer(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->graphicsQueue,
			vkWindow->transferCommandPool,
			vkWindow->transferFence,
			&vkWindow->stagingBuffer,
			&vkWindow->stagingAllocation,
			&vkWindow->stagingSize,
			0,
			window,
			type,
			data,
			size,
			isConstant,
			window->useRayTracing);
#else
	abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		buffer = createGlBuffer(
			window,
			type,
			data,
			size,
			isConstant);
	}
	else
	{
		abort();
	}

	if (buffer == NULL)
		return NULL;

	size_t count = window->bufferCount;

	if (count == window->bufferCapacity)
	{
		size_t capacity = window->bufferCapacity * 2;

		Buffer* buffers = realloc(
			window->buffers,
			sizeof(Buffer) * capacity);

		if (buffers == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkBuffer(
					window->vkWindow->allocator,
					buffer);
#else
				abort();
#endif
			}
			else if (api == OPENGL_GRAPHICS_API ||
				api == OPENGL_ES_GRAPHICS_API)
			{
				destroyGlBuffer(buffer);
			}
			else
			{
				abort();
			}

			return NULL;
		}

		window->buffers = buffers;
		window->bufferCapacity = capacity;
	}

	window->buffers[count] = buffer;
	window->bufferCount = count + 1;
	return buffer;
}
void destroyBuffer(Buffer buffer)
{
	if (buffer == NULL)
		return;

	assert(buffer->base.window->isRecording == false);

	Window window = buffer->base.window;
	Buffer* buffers = window->buffers;
	size_t bufferCount = window->bufferCount;

	for (size_t i = 0; i < bufferCount; i++)
	{
		if (buffer != buffers[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
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
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlBuffer(buffer);
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
	assert(buffer != NULL);
	return buffer->base.window;
}
BufferType getBufferType(Buffer buffer)
{
	assert(buffer != NULL);
	return buffer->base.type;
}
size_t getBufferSize(Buffer buffer)
{
	assert(buffer != NULL);
	return buffer->base.size;
}
bool isBufferConstant(Buffer buffer)
{
	assert(buffer != NULL);
	return buffer->base.isConstant;
}

void* mapBuffer(
	Buffer buffer,
	bool readAccess,
	bool writeAccess)
{
	assert(buffer != NULL);
	assert(buffer->base.isConstant == false);
	assert(buffer->base.isMapped == false);
	assert(readAccess == true || writeAccess == true);

	Window window = buffer->base.window;
	GraphicsAPI api = window->api;

	void* mappedData;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		mappedData = mapVkBuffer(
			window->vkWindow->allocator,
			buffer->vk.allocation,
			readAccess);

		buffer->vk.writeAccess = writeAccess;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		mappedData = mapGlBuffer(
			buffer->gl.glType,
			buffer->gl.handle,
			buffer->gl.size,
			readAccess,
			writeAccess);
	}
	else
	{
		abort();
	}

	if (mappedData == NULL)
		return NULL;

	buffer->base.isMapped = true;
	return mappedData;
}
void unmapBuffer(Buffer buffer)
{
	assert(buffer != NULL);
	assert(buffer->base.isMapped == true);

	Window window = buffer->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		unmapVkBuffer(
			window->vkWindow->allocator,
			buffer->vk.allocation,
			buffer->vk.writeAccess);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		unmapGlBuffer(
			buffer->gl.glType,
			buffer->gl.handle);
	}
	else
	{
		abort();
	}

	buffer->base.isMapped = false;
}

void setBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	assert(buffer != NULL);
	assert(data != NULL);
	assert(size != 0);
	assert(buffer->base.isConstant == false);
	assert(buffer->base.isMapped == false);
	assert(size + offset <= buffer->base.size);

	Window window = buffer->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		setVkBufferData(
			window->vkWindow->allocator,
			buffer->vk.allocation,
			data,
			size,
			offset);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		setGlBufferData(
			buffer->gl.glType,
			buffer->gl.handle,
			data,
			size,
			offset);
	}
	else
	{
		abort();
	}
}

ImageData createImageData(
	const void* data,
	size_t size,
	uint8_t channelCount)
{
	assert(data != NULL);
	assert(size != 0);
	assert(channelCount <= 4);

	ImageData imageData = malloc(sizeof(ImageData_T));

	if (imageData == NULL)
		return NULL;

	stbi_set_flip_vertically_on_load(true);

	int width, height, channels;

	stbi_uc* pixels = stbi_load_from_memory(
		data,
		(int)size,
		&width,
		&height,
		&channels,
		channelCount);

	if (pixels == NULL)
	{
		free(imageData);
		return NULL;
	}

	if (channels != channelCount)
	{
		stbi_image_free(pixels);
		free(imageData);
		return NULL;
	}

	imageData->pixels = pixels;
	imageData->size = vec2U(width, height);
	imageData->channelCount = channelCount;
	return imageData;
}
ImageData createImageDataFromFile(
	const char* filePath,
	uint8_t channelCount)
{
	assert(filePath != NULL);
	assert(channelCount <= 4);

	ImageData imageData = malloc(sizeof(ImageData_T));

	if (imageData == NULL)
		return NULL;

	stbi_set_flip_vertically_on_load(true);

	int width, height, channels;

	stbi_uc* pixels = stbi_load(
		filePath,
		&width,
		&height,
		&channels,
		channelCount);

	if (pixels == NULL)
	{
		free(imageData);
		return NULL;
	}

	if (channels != channelCount)
	{
		stbi_image_free(pixels);
		free(imageData);
		return NULL;
	}

	imageData->pixels = pixels;
	imageData->size = vec2U(width, height);
	imageData->channelCount = channelCount;
	return imageData;
}
void destroyImageData(ImageData imageData)
{
	if (imageData == NULL)
		return;

	stbi_image_free(imageData->pixels);
	free(imageData);
}

const uint8_t* getImageDataPixels(ImageData imageData)
{
	assert(imageData != NULL);
	return imageData->pixels;
}
Vec2U getImageDataSize(ImageData imageData)
{
	assert(imageData != NULL);
	return imageData->size;
}
uint8_t getImageDataChannelCount(ImageData imageData)
{
	assert(imageData != NULL);
	return imageData->channelCount;
}

Image createImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3U size,
	uint8_t levelCount,
	bool isConstant)
{
	assert(window != NULL);
	assert(type < IMAGE_TYPE_COUNT);
	assert(dimension < IMAGE_DIMENSION_COUNT);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(data != NULL);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(levelCount <= getImageLevelCount(size));
	assert(window->isRecording == false);

	GraphicsAPI api = window->api;

	Image image;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		image = createVkImage(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->graphicsQueue,
			vkWindow->transferCommandPool,
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
			levelCount,
			isConstant);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		assert(type != STORAGE_IMAGE_TYPE);

		image = createGlImage(
			window,
			type,
			dimension,
			format,
			data,
			size,
			levelCount,
			isConstant);
	}
	else
	{
		abort();
	}

	if (image == NULL)
		return NULL;

	size_t count = window->imageCount;

	if (count == window->imageCapacity)
	{
		size_t capacity = window->imageCapacity * 2;

		Image* images = realloc(
			window->images,
			sizeof(Image) * capacity);

		if (images == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkImage(
					vkWindow->device,
					vkWindow->allocator,
					image);
#else
				abort();
#endif
			}
			else if (api == OPENGL_GRAPHICS_API ||
				api == OPENGL_ES_GRAPHICS_API)
			{
				destroyGlImage(image);
			}
			else
			{
				abort();
			}

			return NULL;
		}

		window->images = images;
		window->imageCapacity = capacity;
	}

	window->images[count] = image;
	window->imageCount = count + 1;
	return image;
}
Image createImageFromFile(
	Window window,
	ImageFormat format,
	const char* filePath,
	bool generateMipmap,
	bool isConstant)
{
	assert(window != NULL);
	assert(filePath != NULL);
	assert(window->isRecording == false);

	if (format != R8G8B8A8_UNORM_IMAGE_FORMAT &&
		format != R8G8B8A8_SRGB_IMAGE_FORMAT)
	{
		return NULL;
	}

	stbi_set_flip_vertically_on_load(true);

	int width, height, components;

	stbi_uc* pixels = stbi_load(
		filePath,
		&width,
		&height,
		&components,
		4);

	if (pixels == NULL || components != 4)
		return NULL;

	Image image = createImage(
		window,
		GENERAL_IMAGE_TYPE,
		IMAGE_2D,
		format,
		(const void**)&pixels,
		vec3U(width, height, 1),
		generateMipmap ? 0 : 1,
		isConstant);

	stbi_image_free(pixels);
	return image;
}
Image createImageFromData(
	Window window,
	ImageFormat format,
	const void* data,
	size_t size,
	bool generateMipmap,
	bool isConstant)
{
	assert(window != NULL);
	assert(data != NULL);
	assert(size != 0);
	assert(window->isRecording == false);

	if (format != R8G8B8A8_UNORM_IMAGE_FORMAT &&
		format != R8G8B8A8_SRGB_IMAGE_FORMAT)
	{
		return NULL;
	}

	stbi_set_flip_vertically_on_load(true);

	int width, height, components;

	stbi_uc* pixels = stbi_load_from_memory(
		data,
		(int)size,
		&width,
		&height,
		&components,
		4);

	if (pixels == NULL ||
		components != 4)
	{
		return NULL;
	}

	Image image = createImage(
		window,
		GENERAL_IMAGE_TYPE,
		IMAGE_2D,
		format,
		(const void**)&pixels,
		vec3U(width, height, 1),
		generateMipmap ? 0 : 1,
		isConstant);

	stbi_image_free(pixels);
	return image;
}
void destroyImage(Image image)
{
	if (image == NULL)
		return;

	assert(image->base.window->isRecording == false);

	Window window = image->base.window;
	Image* images = window->images;
	size_t imageCount = window->imageCount;

	for (size_t i = 0; i < imageCount; i++)
	{
		if (image != images[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
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
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlImage(image);
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

void setImageData(
	Image image,
	const void* data,
	Vec3U size,
	Vec3U offset)
{
	assert(image != NULL);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(size.x + offset.x <= image->base.size.x);
	assert(size.y + offset.y <= image->base.size.y);
	assert(size.z + offset.z <= image->base.size.z);
	assert(image->base.isConstant == false);
	assert(image->base.window->isRecording == false);

	GraphicsAPI api = image->base.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow =
			image->vk.window->vkWindow;

		setVkImageData(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->graphicsQueue,
			vkWindow->transferCommandPool,
			vkWindow->transferFence,
			image->vk.stagingBuffer,
			image->vk.stagingAllocation,
			image->vk.handle,
			image->vk.vkAspect,
			image->vk.sizeMultiplier,
			data,
			size,
			offset);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		setGlImageData(
			image,
			data,
			size,
			offset);
	}
	else
	{
		abort();
	}
}

Window getImageWindow(Image image)
{
	assert(image != NULL);
	return image->base.window;
}
ImageType getImageType(Image image)
{
	assert(image != NULL);
	return image->base.type;
}
ImageDimension getImageDimension(Image image)
{
	assert(image != NULL);
	return image->base.dimension;
}
ImageFormat getImageFormat(Image image)
{
	assert(image != NULL);
	return image->base.format;
}
Vec3U getImageSize(Image image)
{
	assert(image != NULL);
	return image->base.size;
}
bool isImageConstant(Image image)
{
	assert(image != NULL);
	return image->base.isConstant;
}

uint8_t getImageLevelCount(Vec3U imageSize)
{
	uint32_t size = max(
		max(imageSize.x, imageSize.y),
		imageSize.z);
	return (uint8_t)floorf(log2f((float)size)) + 1;
}

Sampler createSampler(
	Window window,
	ImageFilter minImageFilter,
	ImageFilter magImageFilter,
	ImageFilter minMipmapFilter,
	bool useMipmapping,
	ImageWrap imageWrapX,
	ImageWrap imageWrapY,
	ImageWrap imageWrapZ,
	CompareOperator compareOperator,
	bool useCompare,
	Vec2F mipmapLodRange,
	float mipmapLodBias)
{
	assert(window != NULL);
	assert(minImageFilter < IMAGE_FILTER_COUNT);
	assert(magImageFilter < IMAGE_FILTER_COUNT);
	assert(minMipmapFilter < IMAGE_FILTER_COUNT);
	assert(imageWrapX < IMAGE_WRAP_COUNT);
	assert(imageWrapY < IMAGE_WRAP_COUNT);
	assert(imageWrapZ < IMAGE_WRAP_COUNT);
	assert(compareOperator < COMPARE_OPERATOR_COUNT);
	assert(window->isRecording == false);

	GraphicsAPI api = window->api;

	Sampler sampler;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		sampler = createVkSampler(
			window->vkWindow->device,
			window,
			minImageFilter,
			magImageFilter,
			minMipmapFilter,
			useMipmapping,
			imageWrapX,
			imageWrapY,
			imageWrapZ,
			compareOperator,
			useCompare,
			mipmapLodRange,
			mipmapLodBias);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		assert(mipmapLodBias == 0.0f);

		sampler = createGlSampler(
			window,
			minImageFilter,
			magImageFilter,
			minMipmapFilter,
			useMipmapping,
			imageWrapX,
			imageWrapY,
			imageWrapZ,
			compareOperator,
			useCompare,
			mipmapLodRange);
	}
	else
	{
		abort();
	}

	if (sampler == NULL)
		return NULL;

	size_t count = window->samplerCount;

	if (count == window->samplerCapacity)
	{
		size_t capacity = window->samplerCapacity * 2;

		Sampler* samplers = realloc(
			window->samplers,
			sizeof(Sampler) * capacity);

		if (samplers == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkSampler(
					window->vkWindow->device,
					sampler);
#else
				abort();
#endif
			}
			else if (api == OPENGL_GRAPHICS_API ||
				api == OPENGL_ES_GRAPHICS_API)
			{
				destroyGlSampler(sampler);
			}
			else
			{
				abort();
			}

			return NULL;
		}

		window->samplers = samplers;
		window->samplerCapacity = capacity;
	}

	window->samplers[count] = sampler;
	window->samplerCount = count + 1;
	return sampler;
}
void destroySampler(Sampler sampler)
{
	if (sampler == NULL)
		return;

	assert(sampler->base.window->isRecording == false);

	Window window = sampler->base.window;
	Sampler* samplers = window->samplers;
	size_t samplerCount = window->samplerCount;

	for (size_t i = 0; i < samplerCount; i++)
	{
		if (sampler != samplers[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
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
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlSampler(sampler);
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
	assert(sampler != NULL);
	return sampler->base.window;
}
ImageFilter getSamplerMinImageFilter(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.minImageFilter;
}
ImageFilter getSamplerMagImageFilter(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.magImageFilter;
}
ImageFilter getSamplerMinMipmapFilter(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.minMipmapFilter;
}
bool isSamplerUseMipmapping(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.useMipmapping;
}
ImageWrap getSamplerImageWrapX(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.imageWrapX;
}
ImageWrap getSamplerImageWrapY(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.imageWrapY;
}
ImageWrap getSamplerImageWrapZ(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.imageWrapZ;
}
CompareOperator getSamplerCompareOperator(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.compareOperator;
}
bool isSamplerUseCompare(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.useCompare;
}
Vec2F getSamplerMipmapLodRange(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.mipmapLodRange;
}
float getSamplerMipmapLodBias(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->base.mipmapLodBias;
}

Shader createShader(
	Window window,
	ShaderType type,
	const void* code,
	size_t size)
{
	assert(window != NULL);
	assert(type < SHADER_TYPE_COUNT);
	assert(code != NULL);
	assert(size != 0);
	assert(window->isRecording == false);

	GraphicsAPI api = window->api;

	Shader shader;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		shader = createVkShader(
			window->vkWindow->device,
			window,
			type,
			code,
			size);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
			 api == OPENGL_ES_GRAPHICS_API)
	{
		shader = createGlShader(
			window,
			type,
			code,
			size,
			window->api);
	}
	else
	{
		abort();
	}

	if (shader == NULL)
		return NULL;

	size_t count = window->shaderCount;

#ifndef NDEBUG
	MD5_CTX md5Context;
	md5_init(&md5Context);

	md5_update(
		&md5Context,
		code,
		size);

	uint8_t* hash = shader->base.hash;
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

		if (shaders == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkShader(
					window->vkWindow->device,
					shader);
#else
				abort();
#endif
			}
			else if (api == OPENGL_GRAPHICS_API ||
					 api == OPENGL_ES_GRAPHICS_API)
			{
				destroyGlShader(shader);
			}
			else
			{
				abort();
			}

			return NULL;
		}

		window->shaders = shaders;
		window->shaderCapacity = capacity;
	}

	window->shaders[count] = shader;
	window->shaderCount = count + 1;
	return shader;
}
Shader createShaderFromFile(
	Window window,
	ShaderType type,
	const char* filePath)
{
	assert(window != NULL);
	assert(filePath != NULL);

	FILE* file = openFile(
		filePath,
		"rb");

	if (file == NULL)
		return NULL;

	int seekResult = seekFile(
		file,
		0,
		SEEK_END);

	if (seekResult != 0)
	{
		closeFile(file);
		return NULL;
	}

	size_t fileSize = tellFile(file);

	seekResult = seekFile(
		file,
		0,
		SEEK_SET);

	if (seekResult != 0)
	{
		closeFile(file);
		return NULL;
	}

	GraphicsAPI api = window->api;

	char* code;
	size_t readSize;

	if (api == VULKAN_GRAPHICS_API)
	{
		code = malloc(fileSize);

		if (code == NULL)
		{
			closeFile(file);
			return NULL;
		}

		readSize = fread(
			code,
			sizeof(char),
			fileSize,
			file);
	}
	else if (api == OPENGL_GRAPHICS_API ||
			 api == OPENGL_ES_GRAPHICS_API)
	{
		code = malloc(fileSize + 1);

		if (code == NULL)
		{
			closeFile(file);
			return NULL;
		}

		readSize = fread(
			code,
			sizeof(char),
			fileSize,
			file);
		code[fileSize] = '\0';
	}
	else
	{
		abort();
	}

	closeFile(file);

	if (readSize != fileSize)
	{
		free(code);
		return NULL;
	}

	Shader shader = createShader(
		window,
		type,
		code,
		fileSize);

	free(code);
	return shader;
}
void destroyShader(Shader shader)
{
	if (shader == NULL)
		return;

	assert(shader->base.window->isRecording == false);

	Window window = shader->base.window;
	Shader* shaders = window->shaders;
	size_t shaderCount = window->shaderCount;

	for (size_t i = 0; i < shaderCount; i++)
	{
		if (shader != shaders[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkShader(
				window->vkWindow->device,
				shader);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlShader(shader);
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
	assert(shader != NULL);
	return shader->base.window;
}
ShaderType getShaderType(Shader shader)
{
	assert(shader != NULL);
	return shader->base.type;
}

inline static bool addWindowFramebuffer(
	Window window,
	Framebuffer framebuffer)
{
	size_t count = window->framebufferCount;

	if (count == window->framebufferCapacity)
	{
		size_t capacity = window->framebufferCapacity * 2;

		Framebuffer* framebuffers = realloc(
			window->framebuffers,
			sizeof(Framebuffer) * capacity);

		if (framebuffers == NULL)
			return false;

		window->framebuffers = framebuffers;
		window->framebufferCapacity = capacity;
	}

	window->framebuffers[count] = framebuffer;
	window->framebufferCount = count + 1;
	return true;
}
Framebuffer createFramebuffer(
	Window window,
	Vec2U size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity)
{
	assert(window != NULL);
	assert(size.x != 0 && size.y != 0);
	assert(pipelineCapacity != 0);
	assert(window->isRecording == false);

#ifndef NDEBUG
	bool hasSomeAttachments = false;

	if (colorAttachmentCount != 0)
	{
		assert(colorAttachments != NULL);
		hasSomeAttachments = true;
	}
	if (depthStencilAttachment != NULL)
	{
		assert(depthStencilAttachment->base.type == ATTACHMENT_IMAGE_TYPE);
		assert(depthStencilAttachment->base.size.x == size.x &&
			depthStencilAttachment->base.size.y == size.y);
		assert(depthStencilAttachment->base.window == window);

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image image = colorAttachments[i];

			assert(image->base.type == ATTACHMENT_IMAGE_TYPE);
			assert(image->base.size.x  == size.x &&
				image->base.size.y == size.y);
			assert(image->base.window == window);
		}

		hasSomeAttachments = true;
	}

	assert(hasSomeAttachments == true);
#endif

	GraphicsAPI api = window->api;
	Framebuffer framebuffer;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VkDevice device = vkWindow->device;

		VkRenderPass renderPass = createVkGeneralRenderPass(
			device,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);

		if (renderPass == NULL)
			return NULL;

		framebuffer = createVkFramebuffer(
			device,
			renderPass,
			window,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment,
			pipelineCapacity);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		framebuffer = createGlFramebuffer(
			window,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment,
			pipelineCapacity);
	}
	else
	{
		abort();
	}

	if (framebuffer == NULL)
		return NULL;

	bool result = addWindowFramebuffer(
		window,
		framebuffer);

	if (result == false)
	{
		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(
				window->vkWindow->device,
				framebuffer,
				false);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlFramebuffer(
				framebuffer,
				false);
		}
		else
		{
			abort();
		}

		return NULL;
	}

	return framebuffer;
}
Framebuffer createShadowFramebuffer(
	Window window,
	Vec2U size,
	bool useClear,
	Image depthAttachment,
	size_t pipelineCapacity)
{
	assert(window != NULL);
	assert(size.x != 0 && size.y != 0);
	assert(depthAttachment != NULL);
	assert(depthAttachment->base.size.x == size.x &&
		depthAttachment->base.size.y == size.y);
	assert(depthAttachment->base.window == window);
	assert(pipelineCapacity != 0);
	assert(window->isRecording == false);

	GraphicsAPI api = window->api;
	Framebuffer framebuffer;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VkDevice device = vkWindow->device;

		VkRenderPass renderPass = createVkShadowRenderPass(
			device,
			depthAttachment->vk.vkFormat);

		if (renderPass == NULL)
			return NULL;

		framebuffer = createVkFramebuffer(
			device,
			renderPass,
			window,
			size,
			useClear,
			NULL,
			0,
			depthAttachment,
			pipelineCapacity);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		framebuffer = createGlFramebuffer(
			window,
			size,
			useClear,
			NULL,
			0,
			depthAttachment,
			pipelineCapacity);
	}
	else
	{
		abort();
	}

	if (framebuffer == NULL)
		return NULL;

	bool result = addWindowFramebuffer(
		window,
		framebuffer);

	if (result == false)
	{
		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(
				window->vkWindow->device,
				framebuffer,
				false);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlFramebuffer(
				framebuffer,
				false);
		}
		else
		{
			abort();
		}

		return NULL;
	}

	return framebuffer;
}
void destroyFramebuffer(
	Framebuffer framebuffer,
	bool destroyAttachments)
{
	if (framebuffer == NULL)
		return;

	assert(framebuffer->base.window->isRecording == false);

	Window window = framebuffer->base.window;
	Framebuffer* framebuffers = window->framebuffers;
	size_t framebufferCount = window->framebufferCount;

	for (size_t i = 0; i < framebufferCount; i++)
	{
		if (framebuffer != framebuffers[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkFramebuffer(
				vkWindow->device,
				framebuffer,
				destroyAttachments);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlFramebuffer(
				framebuffer,
				destroyAttachments);
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
	assert(framebuffer != NULL);
	return framebuffer->base.window;
}
Vec2U getFramebufferSize(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	return framebuffer->base.size;
}
bool isFramebufferUseBeginClear(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	return framebuffer->base.useBeginClear;
}
Image* getFramebufferColorAttachments(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	assert(framebuffer->base.isDefault == false);
	return framebuffer->base.colorAttachments;
}
size_t getFramebufferColorAttachmentCount(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	assert(framebuffer->base.isDefault == false);
	return framebuffer->base.colorAttachmentCount;
}
Image getFramebufferDepthStencilAttachment(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	assert(framebuffer->base.isDefault == false);
	return framebuffer->base.depthStencilAttachment;
}
bool isFramebufferDefault(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	return framebuffer->base.isDefault;
}

bool setFramebufferAttachments(
	Framebuffer framebuffer,
	Vec2U size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	assert(framebuffer != NULL);
	assert(size.x != 0 && size.y != 0);
	assert(framebuffer->base.isDefault == false);
	assert(framebuffer->base.window->isRecording == false);

#ifndef NDEBUG
	bool hasSomeAttachments = false;

	if (colorAttachmentCount != 0)
	{
		assert(colorAttachments != NULL);
		hasSomeAttachments = true;
	}
	if (depthStencilAttachment != NULL)
	{
		assert(depthStencilAttachment->base.size.x == size.x &&
			   depthStencilAttachment->base.size.y == size.y);
		assert(depthStencilAttachment->base.window ==
			framebuffer->base.window);
		hasSomeAttachments = true;
	}

	assert(hasSomeAttachments == true);
#endif

	Window window = framebuffer->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		VkResult vkResult = vkQueueWaitIdle(
			vkWindow->graphicsQueue);

		if (vkResult != VK_SUCCESS)
			return false;

		VkDevice device = vkWindow->device;

		VkRenderPass renderPass = createVkGeneralRenderPass(
			device,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);

		if (renderPass == NULL)
			return false;

		bool result = setVkFramebufferAttachments(
			vkWindow->device,
			renderPass,
			framebuffer,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);

		if (result == false)
		{
			vkDestroyRenderPass(
				device,
				renderPass,
				NULL);
			return false;
		}

		return true;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return setGlFramebufferAttachments(
			framebuffer,
			size,
			useBeginClear,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);
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
	assert(framebuffer != NULL);
	assert((clearValues != NULL && clearValueCount != 0 &&
		framebuffer->base.useBeginClear == true) ||
		(clearValues == NULL && clearValueCount == 0 &&
		framebuffer->base.useBeginClear == false));
	assert(framebuffer->base.window->isRecording == true);
	assert(framebuffer->base.window->renderFramebuffer == NULL);

#ifndef NDEBUG
	if (clearValueCount != 0)
	{
		Image depthStencilAttachment =
			framebuffer->base.depthStencilAttachment;
		size_t colorAttachmentCount =
			framebuffer->base.colorAttachmentCount;
		size_t attachmentCount = depthStencilAttachment != NULL ?
			colorAttachmentCount + 1 : colorAttachmentCount;

		if (framebuffer->base.isDefault == false)
			assert(clearValueCount == attachmentCount);
		else
			assert(clearValueCount == 2);

		if (depthStencilAttachment != NULL)
		{
			float depth = clearValues[colorAttachmentCount].depthStencil.depth;
			assert(depth >= 0.0f && depth <= 1.0f);
		}
	}
#endif

	Window window = framebuffer->base.window;
	bool hasDepthBuffer, hasStencilBuffer;

	if (framebuffer->gl.isDefault == false)
	{
		Image depthStencilAttachment =
			framebuffer->gl.depthStencilAttachment;

		if (depthStencilAttachment != NULL)
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
	else
	{
		hasDepthBuffer = true;
		hasStencilBuffer = window->useStencilBuffer;
	}

	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
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
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		bool hasDepthStencilAttachment =
			framebuffer->gl.isDefault == true ||
			framebuffer->gl.depthStencilAttachment != NULL;

		beginGlFramebufferRender(
			framebuffer->gl.handle,
			framebuffer->gl.size,
			framebuffer->gl.colorAttachmentCount,
			hasDepthBuffer,
			hasStencilBuffer,
			clearValues,
			clearValueCount);
	}
	else
	{
		abort();
	}

	window->renderFramebuffer = framebuffer;
}

void endFramebufferRender(
	Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	assert(framebuffer->base.window->isRecording == true);
	assert(framebuffer->base.window->renderFramebuffer == framebuffer);

	Window window = framebuffer->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		endVkFramebufferRender(
			window->vkWindow->currenCommandBuffer);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		endGlFramebufferRender();
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
	assert(framebuffer != NULL);
	assert(clearAttachments != NULL);
	assert(clearValues != NULL);
	assert(clearValueCount != 0);
	assert(framebuffer->base.window->isRecording == true);
	assert(framebuffer->base.window->renderFramebuffer != NULL);

#ifndef NDEBUG
	if (clearValueCount != 0)
	{
		Image depthStencilAttachment =
			framebuffer->base.depthStencilAttachment;
		size_t colorAttachmentCount =
			framebuffer->base.colorAttachmentCount;
		size_t attachmentCount = depthStencilAttachment != NULL ?
			colorAttachmentCount + 1 : colorAttachmentCount;

		if (framebuffer->base.isDefault == false)
			assert(clearValueCount == attachmentCount);
		else
			assert(clearValueCount == 2);

		if (depthStencilAttachment != NULL)
		{
			float depth = clearValues[colorAttachmentCount].depthStencil.depth;
			assert(depth >= 0.0f && depth <= 1.0f);
		}
	}
#endif

	Window window = framebuffer->base.window;
	bool hasDepthBuffer, hasStencilBuffer;

	if (framebuffer->gl.isDefault == false)
	{
		Image depthStencilAttachment =
			framebuffer->gl.depthStencilAttachment;

		if (depthStencilAttachment != NULL)
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
	else
	{
		hasDepthBuffer = true;
		hasStencilBuffer = window->useStencilBuffer;
	}

	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		clearVkFramebuffer(
			window->vkWindow->currenCommandBuffer,
			framebuffer->vk.size,
			hasDepthBuffer,
			hasStencilBuffer,
			clearAttachments,
			clearValues,
			clearValueCount);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		clearGlFramebuffer(
			framebuffer->vk.size,
			framebuffer->gl.colorAttachmentCount,
			hasDepthBuffer,
			hasStencilBuffer,
			clearAttachments,
			clearValues,
			clearValueCount);
	}
	else
	{
		abort();
	}
}

Pipeline createPipeline(
	Framebuffer framebuffer,
	const char* name,
	const PipelineState* state,
	OnPipelineBind onBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineResize onResize,
	OnPipelineDestroy onDestroy,
	void* handle,
	const void* createInfo,
	Shader* shaders,
	size_t shaderCount)
{
	assert(framebuffer != NULL);
	assert(name != NULL);
	assert(state != NULL);
	assert(onResize != NULL);
	assert(onDestroy != NULL);
	assert(handle != NULL);
	assert(shaders != NULL);
	assert(shaderCount != 0);
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
	assert(state->viewport.z >= 0 && state->viewport.w >= 0);
	assert(state->scissor.z >= 0 && state->scissor.w >= 0);
	assert(framebuffer->base.window->isRecording == false);

#ifndef NDEBUG
	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader shader = shaders[i];

		assert(shader->base.type == VERTEX_SHADER_TYPE ||
			shader->base.type == FRAGMENT_SHADER_TYPE ||
			shader->base.type == COMPUTE_SHADER_TYPE ||
			shader->base.type == TESSELLATION_CONTROL_SHADER_TYPE ||
			shader->base.type == TESSELLATION_EVALUATION_SHADER_TYPE ||
			shader->base.type == GEOMETRY_SHADER_TYPE);
		assert(shader->base.window == framebuffer->base.window);
	}
#endif

	Window window = framebuffer->base.window;
	GraphicsAPI api = window->api;

	Pipeline pipeline;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		assert(createInfo != NULL);

		pipeline = createVkPipeline(
			window->vkWindow->device,
			createInfo,
			framebuffer,
			name,
			*state,
			onBind,
			onUniformsSet,
			onResize,
			onDestroy,
			handle,
			shaders,
			shaderCount);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		assert(createInfo == NULL);
		assert(state->discardRasterizer == false);

		pipeline = createGlPipeline(
			framebuffer,
			name,
			*state,
			onBind,
			onUniformsSet,
			onResize,
			onDestroy,
			handle,
			shaders,
			shaderCount);
	}
	else
	{
		abort();
	}

	if (pipeline == NULL)
	{
		onDestroy(handle);
		return NULL;
	}

	size_t count = framebuffer->base.pipelineCount;

	if (count == framebuffer->base.pipelineCapacity)
	{
		size_t capacity = framebuffer->base.pipelineCapacity * 2;

		Pipeline* pipelines = realloc(
			framebuffer->base.pipelines,
			sizeof(Pipeline) * capacity);

		if (pipelines == NULL)
		{
			onDestroy(handle);

			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkPipeline(
					window->vkWindow->device,
					pipeline,
					false);
#else
				abort();
#endif
			}
			else if (api == OPENGL_GRAPHICS_API ||
				api == OPENGL_ES_GRAPHICS_API)
			{
				destroyGlPipeline(
					pipeline,
					false);
			}
			else
			{
				abort();
			}

			return NULL;
		}

		framebuffer->base.pipelines = pipelines;
		framebuffer->base.pipelineCapacity = capacity;
	}

	framebuffer->base.pipelines[count] = pipeline;
	framebuffer->base.pipelineCount = count + 1;
	return pipeline;
}
void destroyPipeline(
	Pipeline pipeline,
	bool destroyShaders)
{
	if (pipeline == NULL)
		return;

	assert(pipeline->base.framebuffer->
		base.window->isRecording == false);

	Framebuffer framebuffer = pipeline->base.framebuffer;
	Window window = framebuffer->base.window;
	size_t pipelineCount = framebuffer->base.pipelineCount;
	Pipeline* pipelines = framebuffer->base.pipelines;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		if (pipeline != pipelines[i])
			continue;

		pipeline->base.onDestroy(
			pipeline->base.handle);

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkPipeline(
				vkWindow->device,
				pipeline,
				destroyShaders);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlPipeline(
				pipeline,
				destroyShaders);
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < pipelineCount; j++)
			pipelines[j - 1] = pipelines[j];

		framebuffer->base.pipelineCount--;
		return;
	}

	abort();
}

Framebuffer getPipelineFramebuffer(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.framebuffer;
}
const char* getPipelineName(Pipeline pipeline)
{
	assert(pipeline != NULL);
#ifndef NDEBUG
	return pipeline->base.name;
#else
	abort();
#endif
}
Shader* getPipelineShaders(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.shaders;
}
size_t getPipelineShaderCount(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.shaderCount;
}
const PipelineState* getPipelineState(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return &pipeline->base.state;
}
OnPipelineBind getPipelineOnBind(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.onBind;
}
OnPipelineUniformsSet getPipelineOnUniformsSet(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.onUniformsSet;
}
OnPipelineResize getPipelineOnResize(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.onResize;
}
OnPipelineDestroy getPipelineOnDestroy(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.onDestroy;
}
void* getPipelineHandle(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.handle;
}

void bindPipeline(Pipeline pipeline)
{
	assert(pipeline != NULL);

	assert(pipeline->base.framebuffer->
		base.window->isRecording == true);
	assert(pipeline->base.framebuffer ==
		pipeline->base.framebuffer->
		base.window->renderFramebuffer);

	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bindVkPipeline(
			window->vkWindow->currenCommandBuffer,
			pipeline);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		bindGlPipeline(pipeline);
	}
	else
	{
		abort();
	}
}

Mesh createMesh(
	Window window,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	assert(window != NULL);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(window->isRecording == false);

#ifndef NDEBUG
	if (vertexBuffer != NULL)
	{
		assert(vertexBuffer->base.window == window);
		assert(vertexBuffer->base.type == VERTEX_BUFFER_TYPE);
	}
	if (indexBuffer != NULL)
	{
		assert(indexBuffer->base.window == window);
		assert(indexBuffer->base.type == INDEX_BUFFER_TYPE);

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

	GraphicsAPI api = window->api;

	Mesh mesh;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		mesh = createVkMesh(
			window,
			indexType,
			indexCount,
			indexOffset,
			vertexBuffer,
			indexBuffer);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		mesh = createGlMesh(
			window,
			indexType,
			indexCount,
			indexOffset,
			vertexBuffer,
			indexBuffer);
	}
	else
	{
		abort();
	}

	if (mesh == NULL)
		return NULL;

	size_t count = window->meshCount;

	if (count == window->meshCapacity)
	{
		size_t capacity = window->meshCapacity * 2;

		Mesh* meshes = realloc(
			window->meshes,
			sizeof(Mesh) * capacity);

		if (meshes == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				destroyVkMesh(
					mesh,
					false);
#else
				abort();
#endif
			}
			else if (api == OPENGL_GRAPHICS_API ||
					 api == OPENGL_ES_GRAPHICS_API)
			{
				destroyGlMesh(
					mesh,
					false);
			}
			else
			{
				abort();
			}

			return NULL;
		}

		window->meshes = meshes;
		window->meshCapacity = capacity;
	}

	window->meshes[count] = mesh;
	window->meshCount = count + 1;
	return mesh;
}
void destroyMesh(
	Mesh mesh,
	bool destroyBuffers)
{
	if (mesh == NULL)
		return;

	assert(mesh->base.window->isRecording == false);

	Window window = mesh->base.window;
	Mesh* meshes = window->meshes;
	size_t meshCount = window->meshCount;

	for (size_t i = 0; i < meshCount; i++)
	{
		if (mesh != meshes[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkMesh(
				mesh,
				destroyBuffers);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlMesh(
				mesh,
				destroyBuffers);
		}
		else
		{
			abort();
		}

		for (size_t j = i + 1; j < meshCount; j++)
			meshes[j - 1] = meshes[j];

		window->meshCount--;
		return;
	}

	abort();
}

Window getMeshWindow(Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->base.window;
}
IndexType getMeshIndexType(Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->base.indexType;
}

size_t getMeshIndexCount(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->base.indexCount;
}
void setMeshIndexCount(
	Mesh mesh,
	size_t indexCount)
{
	assert(mesh != NULL);
	assert(mesh->base.window->isRecording == false);

#ifndef NDEBUG
	if (mesh->base.indexBuffer != NULL)
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

size_t getMeshIndexOffset(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->base.indexOffset;
}
void setMeshIndexOffset(
	Mesh mesh,
	size_t indexOffset)
{
	assert(mesh != NULL);
	assert(mesh->base.window->isRecording == false);

#ifndef NDEBUG
	if (mesh->base.indexBuffer != NULL)
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

	mesh->base.indexOffset = indexOffset;
}

Buffer getMeshVertexBuffer(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->base.vertexBuffer;
}
void setMeshVertexBuffer(
	Mesh mesh,
	Buffer vertexBuffer)
{
	assert(mesh != NULL);
	assert(mesh->base.window->isRecording == false);

#ifndef NDEBUG
	if (vertexBuffer != NULL)
	{
		assert(mesh->base.window == vertexBuffer->base.window);
		assert(vertexBuffer->base.type == VERTEX_BUFFER_TYPE);
	}
#endif

	mesh->base.vertexBuffer = vertexBuffer;
}

Buffer getMeshIndexBuffer(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->base.indexBuffer;
}
void setMeshIndexBuffer(
	Mesh mesh,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer indexBuffer)
{
	assert(mesh != NULL);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(mesh->base.window->isRecording == false);

#ifndef NDEBUG
	if (indexBuffer != NULL)
	{
		assert(mesh->base.window == indexBuffer->base.window);
		assert(indexBuffer->base.type == INDEX_BUFFER_TYPE);

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

	mesh->base.indexType = indexType;
	mesh->base.indexCount = indexCount;
	mesh->base.indexOffset = indexOffset;
	mesh->base.indexBuffer = indexBuffer;
}

size_t drawMesh(
	Mesh mesh,
	Pipeline pipeline)
{
	assert(mesh != NULL);
	assert(pipeline != NULL);
	assert(mesh->base.window == mesh->base.window);
	assert(mesh->base.vertexBuffer->base.isMapped == false);
	assert(mesh->base.indexBuffer->base.isMapped == false);
	assert(mesh->base.window->isRecording == true);

	if (mesh->base.vertexBuffer == NULL ||
		mesh->base.indexBuffer == NULL ||
		mesh->base.indexCount == 0)
	{
		return 0;
	}

	Window window = mesh->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		if (pipeline->base.onUniformsSet != NULL)
			pipeline->base.onUniformsSet(pipeline);

		drawVkMesh(
			window->vkWindow->currenCommandBuffer,
			mesh);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		drawGlMesh(
			mesh,
			pipeline);
	}
	else
	{
		abort();
	}

	return mesh->base.indexCount;
}

RayPipeline createRayPipeline(
	Window window,
	const char* name,
	OnRayPipelineBind onBind,
	OnRayPipelineResize onResize,
	OnRayPipelineDestroy onDestroy,
	void* handle,
	const void* createInfo,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount)
{
	assert(window != NULL);
	assert(name != NULL);
	assert(onResize != NULL);
	assert(onDestroy != NULL);
	assert(handle != NULL);
	assert(createInfo != NULL);
	assert(generationShaders != NULL);
	assert(generationShaderCount != 0);
	assert(missShaders != NULL);
	assert(missShaderCount != 0);
	assert(closestHitShaders != NULL);
	assert(closestHitShaderCount != 0);
	assert(window->useRayTracing == true);
	assert(window->isRecording == false);

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

	GraphicsAPI api = window->api;
	RayTracing rayTracing = window->rayTracing;

	RayPipeline rayPipeline;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		rayPipeline = createVkRayPipeline(
			vkWindow->device,
			vkWindow->allocator,
			createInfo,
			rayTracing,
			name,
			window,
			onBind,
			onResize,
			onDestroy,
			handle,
			generationShaders,
			generationShaderCount,
			missShaders,
			missShaderCount,
			closestHitShaders,
			closestHitShaderCount);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (rayPipeline == NULL)
	{
		onDestroy(handle);
		return NULL;
	}

	size_t count = rayTracing->base.pipelineCount;

	if (count == rayTracing->base.pipelineCapacity)
	{
		size_t capacity = rayTracing->base.pipelineCapacity * 2;

		RayPipeline* pipelines = realloc(
			rayTracing->base.pipelines,
			sizeof(RayPipeline) * capacity);

		if (pipelines == NULL)
		{
			onDestroy(handle);

			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkRayPipeline(
					vkWindow->device,
					vkWindow->allocator,
					rayPipeline,
					false);
#else
				abort();
#endif
			}
			else
			{
				abort();
			}

			return NULL;
		}

		rayTracing->base.pipelines = pipelines;
		rayTracing->base.pipelineCapacity = capacity;
	}

	rayTracing->base.pipelines[count] = rayPipeline;
	rayTracing->base.pipelineCount = count + 1;
	return rayPipeline;
}
void destroyRayPipeline(
	RayPipeline rayPipeline,
	bool destroyShaders)
{
	if (rayPipeline == NULL)
		return;

	assert(rayPipeline->base.window->isRecording == false);

	Window window = rayPipeline->base.window;
	RayTracing rayTracing = window->rayTracing;
	size_t pipelineCount = rayTracing->base.pipelineCount;
	RayPipeline* pipelines = rayTracing->base.pipelines;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		if (rayPipeline != pipelines[i])
			continue;

		rayPipeline->base.onDestroy(
			rayPipeline->base.handle);

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkRayPipeline(
				vkWindow->device,
				vkWindow->allocator,
				rayPipeline,
				destroyShaders);
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

Window getRayPipelineWindow(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.window;
}
const char* getRayPipelineName(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
#ifndef NDEBUG
	return rayPipeline->base.name;
#else
	abort();
#endif
}
Shader* getRayPipelineGenerationShaders(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.generationShaders;
}
size_t getRayPipelineGenerationShaderCount(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.generationShaderCount;
}
Shader* getRayPipelineMissShaders(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.missShaders;
}
size_t getRayPipelineMissShaderCount(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.missShaderCount;
}
Shader* getRayPipelineClosestHitShaders(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.closestHitShaders;
}
size_t getRayPipelineClosestHitShaderCount(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.closestHitShaderCount;
}
OnRayPipelineBind getRayPipelineOnBind(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.onBind;
}
OnRayPipelineResize getRayPipelineOnResize(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.onResize;
}
OnRayPipelineDestroy getRayPipelineOnDestroy(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.onDestroy;
}
void* getRayPipelineHandle(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	return rayPipeline->base.handle;
}

void bindRayPipeline(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	assert(rayPipeline->base.window->isRecording == true);

	Window window = rayPipeline->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bindVkRayPipeline(
			window->vkWindow->currenCommandBuffer,
			rayPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

void tracePipelineRays(RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	assert(rayPipeline->base.window->isRecording == true);

	Window window = rayPipeline->base.window;
	GraphicsAPI api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		traceVkPipelineRays(
			window->vkWindow->currenCommandBuffer,
			window->rayTracing,
			rayPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

RayMesh createRayMesh(
	Window window,
	size_t vertexStride,
	IndexType indexType,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	assert(window != NULL);
	assert(vertexStride != 0);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);
	assert(vertexBuffer->base.type == VERTEX_BUFFER_TYPE);
	assert(indexBuffer->base.type == INDEX_BUFFER_TYPE);
	assert(vertexBuffer->base.window == window);
	assert(indexBuffer->base.window == window);
	assert(vertexBuffer->base.isMapped == false);
	assert(indexBuffer->base.isMapped == false);
	assert(window->useRayTracing == true);
	assert(window->isRecording == false);

	GraphicsAPI api = window->api;
	RayTracing rayTracing = window->rayTracing;

	RayMesh rayMesh;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		rayMesh = createVkRayMesh(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->graphicsQueue,
			vkWindow->transferCommandPool,
			vkWindow->transferFence,
			rayTracing,
			window,
			vertexStride,
			indexType,
			vertexBuffer,
			indexBuffer);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (rayMesh == NULL)
		return NULL;

	size_t count = rayTracing->base.meshCount;

	if (count == rayTracing->base.meshCapacity)
	{
		size_t capacity = rayTracing->base.meshCapacity * 2;

		RayMesh* meshes = realloc(
			rayTracing->base.meshes,
			sizeof(RayMesh) * capacity);

		if (meshes == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkRayMesh(
					vkWindow->device,
					vkWindow->allocator,
					window->rayTracing,
					rayMesh,
					false);
#else
				abort();
#endif
			}
			else
			{
				abort();
			}

			return NULL;
		}

		rayTracing->base.meshes = meshes;
		rayTracing->base.meshCapacity = capacity;
	}

	rayTracing->base.meshes[count] = rayMesh;
	rayTracing->base.meshCount = count + 1;
	return rayMesh;
}
void destroyRayMesh(
	RayMesh rayMesh,
	bool destroyBuffers)
{
	if (rayMesh == NULL)
		return;

	assert(rayMesh->base.window->isRecording == false);

	Window window = rayMesh->base.window;
	RayTracing rayTracing = window->rayTracing;
	RayMesh* meshes = rayTracing->base.meshes;
	size_t meshCount = rayTracing->base.meshCount;

	for (size_t i = 0; i < meshCount; i++)
	{
		if (rayMesh != meshes[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkRayMesh(
				vkWindow->device,
				vkWindow->allocator,
				window->rayTracing,
				rayMesh,
				destroyBuffers);
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

Window getRayMeshWindow(RayMesh rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.window;
}
size_t getRayMeshVertexStride(RayMesh rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.vertexStride;
}
IndexType getRayMeshIndexType(RayMesh rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.indexType;
}
Buffer getRayMeshVertexBuffer(RayMesh rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.vertexBuffer;
}
Buffer getRayMeshIndexBuffer(RayMesh rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.indexBuffer;
}

RayScene createRayScene(
	Window window,
	RayMesh* rayMeshes,
	size_t rayMeshCount)
{
	assert(window != NULL);
	assert(rayMeshes != NULL);
	assert(rayMeshCount != 0);
	assert(window->useRayTracing == true);
	assert(window->isRecording == false);

	GraphicsAPI api = window->api;
	RayTracing rayTracing = window->rayTracing;

	RayScene rayScene;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		rayScene = createVkRayScene(
			vkWindow->device,
			vkWindow->allocator,
			vkWindow->graphicsQueue,
			vkWindow->transferCommandPool,
			vkWindow->transferFence,
			rayTracing,
			window,
			rayMeshes,
			rayMeshCount);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (rayScene == NULL)
		return NULL;

	size_t count = rayTracing->base.sceneCount;

	if (count == rayTracing->base.sceneCapacity)
	{
		size_t capacity = rayTracing->base.sceneCapacity * 2;

		RayScene* scenes = realloc(
			rayTracing->base.scenes,
			sizeof(RayScene) * capacity);

		if (scenes == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
#if MPGX_SUPPORT_VULKAN
				VkWindow vkWindow = window->vkWindow;

				destroyVkRayScene(
					vkWindow->device,
					vkWindow->allocator,
					window->rayTracing,
					rayScene);
#else
				abort();
#endif
			}
			else
			{
				abort();
			}

			return NULL;
		}

		rayTracing->base.scenes = scenes;
		rayTracing->base.sceneCapacity = capacity;
	}

	rayTracing->base.scenes[count] = rayScene;
	rayTracing->base.sceneCount = count + 1;
	return rayScene;
}
void destroyRayScene(RayScene rayScene)
{
	if (rayScene == NULL)
		return;

	assert(rayScene->base.window->isRecording == false);

	Window window = rayScene->base.window;
	RayTracing rayTracing = window->rayTracing;
	RayScene* scenes = rayTracing->base.scenes;
	size_t sceneCount = rayTracing->base.sceneCount;

	for (size_t i = 0; i < sceneCount; i++)
	{
		if (rayScene != scenes[i])
			continue;

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (result != VK_SUCCESS)
				abort();

			destroyVkRayScene(
				vkWindow->device,
				vkWindow->allocator,
				window->rayTracing,
				rayScene);
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

Window getRaySceneWindow(RayScene rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.window;
}
RayMesh* getRaySceneMeshes(RayScene rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.meshes;
}
size_t getRaySceneMeshCount(RayScene rayMesh)
{
	assert(rayMesh != NULL);
	return rayMesh->base.meshCount;
}
