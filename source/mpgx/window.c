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
#include "mpgx/file.h"
#include "mpgx/_source/mesh.h"
#include "mpgx/_source/sampler.h"
#include "mpgx/_source/framebuffer.h"

#include "cmmt/common.h"
#include "mpmt/thread.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>

// TODO: possibly create Vulkan window staging buffer
// recreate if staging more data, and stage/destroy on frame render end.

struct ImageData
{
	uint8_t* pixels;
	Vec2U size;
	uint8_t channelCount;
};

struct Window
{
	GraphicsAPI api;
	bool useStencilBuffer;
	OnWindowUpdate onUpdate;
	void* updateArgument;
	GLFWwindow* handle;
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
	fprintf(stderr,
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
bool initializeGraphics(
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch)
{
	assert(appName != NULL);
	assert(graphicsInitialized == false);

	if(glfwInit() == GLFW_FALSE)
		return false;

	glfwSetErrorCallback(glfwErrorCallback);

	if (FT_Init_FreeType(&ftLibrary) != 0)
	{
		glfwTerminate();
		return false;
	}

#if MPGX_SUPPORT_VULKAN
	const char* preferredLayers[1] = {
		"VK_LAYER_KHRONOS_validation",
	};
	const char* preferredExtensions[1] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

	bool supportedExtensions[1];

	vkInstance = createVkInstance(
		appName,
		appVersionMajor,
		appVersionMinor,
		appVersionPatch,
		NULL,
		0,
		preferredLayers,
		1,
		NULL,
		0,
		preferredExtensions,
		1,
		supportedExtensions);

	if (vkInstance == NULL)
	{
		terminateFreeTypeLibrary(ftLibrary);
		glfwTerminate();
		return false;
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
		return false;
	}
#endif

#endif

	graphicsInitialized = true;
	return true;
}
void terminateGraphics()
{
	assert(graphicsInitialized == true);

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

Window createWindow(
	GraphicsAPI api,
	bool useStencilBuffer,
	Vec2U size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool isVisible,
	size_t bufferCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t shaderCapacity,
	size_t framebufferCapacity,
	size_t pipelineCapacity,
	size_t meshCapacity)
{
	assert(api < GRAPHICS_API_COUNT);
	assert(size.x != 0);
	assert(size.y != 0);
	assert(title != NULL);
	assert(onUpdate != NULL);
	assert(bufferCapacity != 0);
	assert(imageCapacity != 0);
	assert(samplerCapacity != 0);
	assert(shaderCapacity != 0);
	assert(framebufferCapacity != 0);
	assert(pipelineCapacity != 0);
	assert(meshCapacity != 0);
	assert(graphicsInitialized == true);

	glfwDefaultWindowHints();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		if (glfwVulkanSupported() == GLFW_FALSE)
			return NULL;

		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_NO_API);
#else
		return NULL;
#endif
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
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

	glfwWindowHint(
		GLFW_VISIBLE,
		isVisible ? GLFW_TRUE : GLFW_FALSE);

	Window window = malloc(
		sizeof(struct Window));

	if (window == NULL)
		return NULL;

	GLFWwindow* handle = glfwCreateWindow(
		(int)size.x,
		(int)size.y,
		title,
		NULL,
		NULL);

	if (handle == NULL)
	{
		free(window);
		return NULL;
	}

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

	Framebuffer framebuffer;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		vkWindow = createVkWindow(
			window,
			vkInstance,
			handle,
			useStencilBuffer,
			framebufferSize);

		if (vkWindow == NULL)
		{
			glfwDestroyWindow(handle);
			free(window);
			return NULL;
		}

		VkSwapchain swapchain = vkWindow->swapchain;
		VkSwapchainBuffer firtBuffer = swapchain->buffers[0];

		VkImageView imageViews[2] = {
			firtBuffer.imageView,
			swapchain->depthImageView,
		};

		framebuffer = createDefaultVkFramebuffer(
			swapchain->renderPass,
			imageViews,
			firtBuffer.framebuffer,
			window,
			framebufferSize,
			1,
			pipelineCapacity);

		if (framebuffer == NULL)
		{
			destroyVkWindow(vkInstance, vkWindow);
			glfwDestroyWindow(handle);
			free(window);
			return NULL;
		}
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
			glfwDestroyWindow(handle);
			free(window);
			return NULL;
		}

		glEnable(GL_FRAMEBUFFER_SRGB);

		framebuffer = createDefaultGlFramebuffer(
			window,
			framebufferSize,
			pipelineCapacity);

		if (framebuffer == NULL)
		{
			glfwDestroyWindow(handle);
			free(window);
			return NULL;
		}
	}

	Buffer* buffers = malloc(
		sizeof(Buffer) * bufferCapacity);

	if (buffers == NULL)
	{
		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(vkWindow->device, framebuffer);
			destroyVkWindow(vkInstance, vkWindow);
#endif
		}
		else
		{
			destroyGlFramebuffer(framebuffer);
		}

		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Image* images = malloc(
		sizeof(Image) * imageCapacity);

	if (images == NULL)
	{
		free(buffers);

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(vkWindow->device, framebuffer);
			destroyVkWindow(vkInstance, vkWindow);
#endif
		}
		else
		{
			destroyGlFramebuffer(framebuffer);
		}

		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Sampler* samplers = malloc(
		sizeof(Sampler) * samplerCapacity);

	if (samplers == NULL)
	{
		free(images);
		free(buffers);

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(vkWindow->device, framebuffer);
			destroyVkWindow(vkInstance, vkWindow);
#endif
		}
		else
		{
			destroyGlFramebuffer(framebuffer);
		}

		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Shader* shaders = malloc(
		sizeof(Shader) * shaderCapacity);

	if (shaders == NULL)
	{
		free(samplers);
		free(images);
		free(buffers);

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(vkWindow->device, framebuffer);
			destroyVkWindow(vkInstance, vkWindow);
#endif
		}
		else
		{
			destroyGlFramebuffer(framebuffer);
		}

		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Framebuffer* framebuffers = malloc(
		sizeof(Framebuffer) * framebufferCapacity);

	if (framebuffers == NULL)
	{
		free(shaders);
		free(samplers);
		free(images);
		free(buffers);

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(vkWindow->device, framebuffer);
			destroyVkWindow(vkInstance, vkWindow);
#endif
		}
		else
		{
			destroyGlFramebuffer(framebuffer);
		}

		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Mesh* meshes = malloc(
		sizeof(Mesh) * meshCapacity);

	if (meshes == NULL)
	{
		free(framebuffers);
		free(shaders);
		free(samplers);
		free(images);
		free(meshes);
		free(buffers);

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkFramebuffer(vkWindow->device, framebuffer);
			destroyVkWindow(vkInstance, vkWindow);
#endif
		}
		else
		{
			destroyGlFramebuffer(framebuffer);
		}

		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	window->api = api;
	window->useStencilBuffer = useStencilBuffer;
	window->onUpdate = onUpdate;
	window->updateArgument = updateArgument;
	window->handle = handle;
	window->framebuffer = framebuffer;
	window->buffers = buffers;
	window->bufferCapacity = bufferCapacity;
	window->bufferCount = 0;
	window->images = images;
	window->imageCapacity = imageCapacity;
	window->imageCount = 0;
	window->samplers = samplers;
	window->samplerCapacity = samplerCapacity;
	window->samplerCount = 0;
	window->shaders = shaders;
	window->shaderCapacity = shaderCapacity;
	window->shaderCount = 0;
	window->framebuffers = framebuffers;
	window->framebufferCapacity = framebufferCapacity;
	window->framebufferCount = 0;
	window->meshes = meshes;
	window->meshCapacity = meshCapacity;
	window->meshCount = 0;
	window->targetFPS = 60.0;
	window->updateTime = 0.0;
	window->deltaTime = 0.0;
	window->isRecording = false;
	window->renderFramebuffer = NULL;
#if MPGX_SUPPORT_VULKAN
	window->vkWindow = vkWindow;
#endif

	currentWindow = window;
	return window;
}
Window createAnyWindow(
	bool useStencilBuffer,
	Vec2U size,
	const char* title,
	OnWindowUpdate updateFunction,
	void* updateArgument,
	bool visible,
	size_t bufferCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t shaderCapacity,
	size_t framebufferCapacity,
	size_t pipelineCapacity,
	size_t meshCapacity)
{
	Window window = createWindow(
		VULKAN_GRAPHICS_API,
		useStencilBuffer,
		size,
		title,
		updateFunction,
		updateArgument,
		visible,
		bufferCapacity,
		imageCapacity,
		samplerCapacity,
		shaderCapacity,
		framebufferCapacity,
		pipelineCapacity,
		meshCapacity);

	if (window != NULL)
		return window;

	window = createWindow(
		OPENGL_GRAPHICS_API,
		useStencilBuffer,
		size,
		title,
		updateFunction,
		updateArgument,
		visible,
		bufferCapacity,
		imageCapacity,
		samplerCapacity,
		shaderCapacity,
		framebufferCapacity,
		pipelineCapacity,
		meshCapacity);

	if (window != NULL)
		return window;

	window = createWindow(
		OPENGL_ES_GRAPHICS_API,
		useStencilBuffer,
		size,
		title,
		updateFunction,
		updateArgument,
		visible,
		bufferCapacity,
		imageCapacity,
		samplerCapacity,
		shaderCapacity,
		framebufferCapacity,
		pipelineCapacity,
		meshCapacity);

	return window;
}
void destroyWindow(Window window)
{
	if (window == NULL)
        return;

	Mesh* meshes = window->meshes;
	size_t meshCount = window->meshCount;
	Shader* shaders = window->shaders;
	size_t shaderCount = window->shaderCount;
	Framebuffer* framebuffers = window->framebuffers;
	size_t framebufferCount = window->framebufferCount;
	Sampler* samplers = window->samplers;
	size_t samplerCount = window->samplerCount;
	Image* images = window->images;
	size_t imageCount = window->imageCount;
	Buffer* buffers = window->buffers;
	size_t bufferCount = window->bufferCount;

	GraphicsAPI api = window->api;

    if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VkDevice device = vkWindow->device;
		VmaAllocator allocator = vkWindow->allocator;

		VkResult result = vkDeviceWaitIdle(
			vkWindow->device);

		if (result != VK_SUCCESS)
			abort();

		for (size_t i = 0; i < meshCount; i++)
			destroyVkMesh(meshes[i]);
		for (size_t i = 0; i < shaderCount; i++)
			destroyVkShader(device, shaders[i]);
		for (size_t i = 0; i < framebufferCount; i++)
			destroyVkFramebuffer(device, framebuffers[i]);
		for (size_t i = 0; i < samplerCount; i++)
			destroyVkSampler(device, samplers[i]);
		for (size_t i = 0; i < imageCount; i++)
			destroyVkImage(device, allocator, images[i]);
		for (size_t i = 0; i < bufferCount; i++)
			destroyVkBuffer(allocator, buffers[i]);

		destroyVkFramebuffer(
			device,
			window->framebuffer);

		destroyVkWindow(
			vkInstance,
			vkWindow);
#endif
	}
    else if (api == OPENGL_GRAPHICS_API ||
    	api == OPENGL_ES_GRAPHICS_API)
	{
		for (size_t i = 0; i < meshCount; i++)
			destroyGlMesh(meshes[i]);
		for (size_t i = 0; i < shaderCount; i++)
			destroyGlShader(shaders[i]);
		for (size_t i = 0; i < framebufferCount; i++)
			destroyGlFramebuffer(framebuffers[i]);
		for (size_t i = 0; i < samplerCount; i++)
			destroyGlSampler(samplers[i]);
		for (size_t i = 0; i < imageCount; i++)
			destroyGlImage(images[i]);
		for (size_t i = 0; i < bufferCount; i++)
			destroyGlBuffer(buffers[i]);

		destroyGlFramebuffer(window->framebuffer);
	}
    else
	{
    	abort();
	}

	free(buffers);
	free(meshes);
	free(images);
	free(framebuffers);
	free(shaders);

	glfwDestroyWindow(window->handle);
	free(window);
}

bool isWindowEmpty(Window window)
{
	assert(window != NULL);

	return
		window->bufferCount == 0 &&
		window->imageCount == 0 &&
		window->samplerCount == 0 &&
		window->shaderCount == 0 &&
		window->framebufferCount == 0 &&
		window->meshCount == 0;
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
const char* getWindowClipboard(Window window)
{
	assert(window != NULL);
	return glfwGetClipboardString(window->handle);
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

Framebuffer getWindowFramebuffer(Window window)
{
	assert(window != NULL);
	return window->framebuffer;
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
	framebuffer->vk.imageViews[0] = firstBuffer.imageView;
	framebuffer->vk.imageViews[1] = swapchain->depthImageView;
	framebuffer->vk.handle = firstBuffer.framebuffer;

	Pipeline* pipelines = framebuffer->vk.pipelines;
	size_t pipelineCount = framebuffer->vk.pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];
		VkPipelineCreateInfo createInfo;

		if (pipeline->vk.onHandleResize != NULL)
		{
			bool result = pipeline->vk.onHandleResize(
				pipeline,
				newSize,
				&createInfo);

			if (result == false)
				return false;
		}

		VkPipeline vkHandle = createVkPipelineHandle(
			swapchain->renderPass,
			pipeline->vk.cache,
			pipeline->vk.layout,
			device,
			framebuffer->vk.window,
			&createInfo,
			pipeline->vk.shaders,
			pipeline->vk.shaderCount,
			pipeline->vk.state);

		if (vkHandle == NULL)
			return false;

		vkDestroyPipeline(
			device,
			pipeline->vk.vkHandle,
			NULL);

		pipeline->vk.vkHandle = vkHandle;
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
				vkWindow->allocator,
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
					vkWindow->allocator,
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
			vkWindow->allocator,
			bufferIndex);

		VkSwapchainBuffer buffer = swapchain->buffers[bufferIndex];
		framebuffer->vk.renderPass = swapchain->renderPass;
		framebuffer->vk.imageViews[0] = buffer.imageView;
		framebuffer->vk.imageViews[1] = swapchain->depthImageView;
		framebuffer->vk.handle = buffer.framebuffer;

		VkCommandBuffer graphicsCommandBuffer = buffer.graphicsCommandBuffer;

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

				if (pipeline->gl.onHandleResize != NULL)
				{
					pipeline->gl.onHandleResize(
						pipeline,
						newSize,
						NULL);
				}
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
				0,
				0,
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
			0,
			window,
			type,
			data,
			size,
			isConstant);
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

			VkResult result = vkDeviceWaitIdle(
				vkWindow->device);

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
	uint8_t _channelCount)
{
	assert(data != NULL);
	assert(size != 0);
	assert(_channelCount <= 4);

	ImageData imageData = malloc(
		sizeof(struct ImageData));

	if (imageData == NULL)
		return NULL;

	stbi_set_flip_vertically_on_load(true);

	int width, height, channelCount;

	stbi_uc* pixels = stbi_load_from_memory(
		data,
		(int)size,
		&width,
		&height,
		&channelCount,
		_channelCount);

	if (pixels == NULL)
	{
		free(imageData);
		return NULL;
	}

	if (channelCount != _channelCount)
	{
		stbi_image_free(pixels);
		free(imageData);
		return NULL;
	}

	imageData->pixels = pixels;
	imageData->size = vec2U(width, height);
	imageData->channelCount = _channelCount;
	return imageData;
}
ImageData createImageDataFromFile(
	const char* filePath,
	uint8_t _channelCount)
{
	assert(filePath != NULL);
	assert(_channelCount <= 4);

	ImageData imageData = malloc(
		sizeof(struct ImageData));

	if (imageData == NULL)
		return NULL;

	stbi_set_flip_vertically_on_load(true);

	int width, height, channelCount;

	stbi_uc* pixels = stbi_load(
		filePath,
		&width,
		&height,
		&channelCount,
		_channelCount);

	if (pixels == NULL)
	{
		free(imageData);
		return NULL;
	}

	if (channelCount != _channelCount)
	{
		stbi_image_free(pixels);
		free(imageData);
		return NULL;
	}

	imageData->pixels = pixels;
	imageData->size = vec2U(width, height);
	imageData->channelCount = _channelCount;
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
	ImageFormat format,
	const void** data,
	Vec3U size,
	uint8_t levelCount,
	bool isConstant,
	bool isAttachment)
{
	assert(window != NULL);
	assert(type < IMAGE_TYPE_COUNT);
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
			window,
			type,
			format,
			data,
			size,
			levelCount,
			isConstant,
			isAttachment);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		image = createGlImage(
			window,
			type,
			format,
			data,
			size,
			levelCount,
			isConstant,
			isAttachment);
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
		IMAGE_2D_TYPE,
		format,
		(const void**)&pixels,
		vec3U(width, height, 1),
		generateMipmap ? 0 : 1,
		isConstant,
		false);

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
		IMAGE_2D_TYPE,
		format,
		(const void**)&pixels,
		vec3U(width, height, 1),
		generateMipmap ? 0 : 1,
		isConstant,
		false);

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

			VkResult result = vkDeviceWaitIdle(
				vkWindow->device);

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
			vkWindow->allocator,
			image->vk.stagingBuffer,
			image->vk.stagingAllocation,
			image->vk.stagingFence,
			vkWindow->device,
			vkWindow->graphicsQueue,
			vkWindow->transferCommandPool,
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

			VkResult result = vkDeviceWaitIdle(
				vkWindow->device);

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

			free(shader);
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
		fclose(file);
		return NULL;
	}

	size_t fileSize = tellFile(file);

	seekResult = seekFile(
		file,
		0,
		SEEK_SET);

	if (seekResult != 0)
	{
		fclose(file);
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
			fclose(file);
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
			fclose(file);
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

	fclose(file);

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
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity)
{
	assert(window != NULL);
	assert(size.x != 0 && size.y != 0);
	assert(pipelineCapacity != 0);
	assert(window->isRecording == false);

	assert((colorAttachments != NULL &&
		colorAttachmentCount != 0) ||
		(depthStencilAttachment != NULL &&
		depthStencilAttachment->base.size.x == size.x &&
		depthStencilAttachment->base.size.y == size.y &&
		depthStencilAttachment->base.window == window));

	GraphicsAPI api = window->api;
	Framebuffer framebuffer;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;
		VkDevice device = vkWindow->device;

		VkRenderPass renderPass = createVkGeneralRenderPass(
			device,
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
				framebuffer);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlFramebuffer(framebuffer);
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
				framebuffer);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlFramebuffer(framebuffer);
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

		if (destroyAttachments == true)
		{
			Image* colorAttachments = framebuffer->base.colorAttachments;
			size_t colorAttachmentCount = framebuffer->base.colorAttachmentCount;

			for (size_t j = 0; j < colorAttachmentCount; j++)
				destroyImage(colorAttachments[j]);

			destroyImage(framebuffer->base.depthStencilAttachment);
		}

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkDeviceWaitIdle(
				vkWindow->device);

			if (result != VK_SUCCESS)
				abort();

			destroyVkFramebuffer(
				vkWindow->device,
				framebuffer);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlFramebuffer(framebuffer);
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
Image* getFramebufferColorAttachments(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	assert(framebuffer->base.isDefault == false);
	return framebuffer->base.colorAttachments;
}
uint8_t getFramebufferColorAttachmentCount(Framebuffer framebuffer)
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
bool isFramebufferEmpty(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	return framebuffer->base.pipelineCount == 0;
}

bool setFramebufferAttachments(
	Framebuffer framebuffer,
	Vec2U size,
	Image* attachments,
	size_t attachmentCount)
{
	assert(framebuffer != NULL);
	assert(attachments != NULL);
	assert(attachmentCount != 0);
	assert(!(framebuffer->base.isDefault == true &&
		(framebuffer->base.window->api == OPENGL_GRAPHICS_API ||
		framebuffer->base.window->api == OPENGL_ES_GRAPHICS_API)));
	assert(framebuffer->base.window->isRecording == false);

	// TODO:
	return false;
}

void beginFramebufferRender(
	Framebuffer framebuffer,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	assert(framebuffer != NULL);
	assert((clearValues != NULL && clearValueCount != 0) || clearValueCount == 0);
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

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			LinearColor color = clearValues[i].color;
			assertLinearColor(color);
		}

		if (depthStencilAttachment != NULL)
		{
			float depth = clearValues[colorAttachmentCount].depthStencil.depth;
			assert(depth >= 0.0f && depth <= 1.0f);
		}
	}
#endif

	Window window = framebuffer->base.window;
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
			hasDepthStencilAttachment,
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
	assert((clearAttachments != NULL && clearValues != NULL &&
		clearValueCount != 0) || clearValueCount == 0);
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

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			LinearColor color = clearValues[i].color;
			assertLinearColor(color);
		}

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
	Shader* shaders,
	size_t shaderCount,
	const PipelineState* state,
	OnPipelineHandleDestroy onHandleDestroy,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineHandleResize onHandleResize,
	void* handle,
	void* createInfo)
{
	assert(framebuffer != NULL);
	assert(name != NULL);
	assert(shaders != NULL);
	assert(shaderCount != 0);
	assert(state != NULL);
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
			shaders,
			shaderCount,
			*state,
			onHandleDestroy,
			onHandleBind,
			onUniformsSet,
			onHandleResize,
			handle);
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
			shaders,
			shaderCount,
			*state,
			onHandleDestroy,
			onHandleBind,
			onUniformsSet,
			onHandleResize,
			handle);
	}
	else
	{
		abort();
	}

	if (pipeline == NULL)
		return NULL;

	size_t count = framebuffer->base.pipelineCount;

	if (count == framebuffer->base.pipelineCapacity)
	{
		size_t capacity = framebuffer->base.pipelineCapacity * 2;

		Pipeline* pipelines = realloc(
			framebuffer->base.pipelines,
			sizeof(Pipeline) * capacity);

		if (pipelines == NULL)
		{
			free(pipeline);
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

		if (pipeline->base.onHandleDestroy != NULL)
		{
			pipeline->base.onHandleDestroy(
				pipeline->base.handle);
		}

		if (destroyShaders == true)
		{
			Shader* shaders = pipeline->base.shaders;
			uint8_t shaderCount = pipeline->base.shaderCount;

			for (uint8_t j = 0; j < shaderCount; j++)
				destroyShader(shaders[j]);
		}

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkDeviceWaitIdle(
				vkWindow->device);

			if (result != VK_SUCCESS)
				abort();

			destroyVkPipeline(
				vkWindow->device,
				pipeline);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlPipeline(pipeline);
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
	return pipeline->base.name;
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
OnPipelineHandleDestroy getPipelineOnHandleDestroy(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.onHandleDestroy;
}
OnPipelineHandleBind getPipelineOnHandleBind(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.onHandleBind;
}
OnPipelineUniformsSet getPipelineOnUniformsSet(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->base.onUniformsSet;
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
	DrawIndex drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	assert(window != NULL);
	assert(drawIndex < DRAW_INDEX_COUNT);
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

		if (drawIndex == UINT16_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint16_t) +
				indexOffset * sizeof(uint16_t) <=
				indexBuffer->base.size);
		}
		else if (drawIndex == UINT32_DRAW_INDEX)
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
			drawIndex,
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
			drawIndex,
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
				destroyVkMesh(mesh);
#else
				abort();
#endif
			}
			else if (api == OPENGL_GRAPHICS_API ||
					 api == OPENGL_ES_GRAPHICS_API)
			{
				destroyGlMesh(mesh);
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

		if (destroyBuffers == true)
		{
			destroyBuffer(mesh->base.vertexBuffer);
			destroyBuffer(mesh->base.indexBuffer);
		}

		GraphicsAPI api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			destroyVkMesh(mesh);
#else
			abort();
#endif
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			destroyGlMesh(mesh);
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
DrawIndex getMeshDrawIndex(Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->base.drawIndex;
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
		if (mesh->base.drawIndex == UINT16_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint16_t) +
				mesh->base.indexOffset * sizeof(uint16_t) <=
				mesh->base.indexBuffer->base.size);
		}
		else if (mesh->base.drawIndex == UINT32_DRAW_INDEX)
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
		if (mesh->base.drawIndex == UINT16_DRAW_INDEX)
		{
			assert(mesh->base.indexCount * sizeof(uint16_t) +
				indexOffset * sizeof(uint16_t) <=
				mesh->base.indexBuffer->base.size);
		}
		else if (mesh->base.drawIndex == UINT32_DRAW_INDEX)
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
	DrawIndex drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer indexBuffer)
{
	assert(mesh != NULL);
	assert(drawIndex < DRAW_INDEX_COUNT);
	assert(mesh->base.window->isRecording == false);

#ifndef NDEBUG
	if (indexBuffer != NULL)
	{
		assert(mesh->base.window == indexBuffer->base.window);
		assert(indexBuffer->base.type == INDEX_BUFFER_TYPE);

		if (drawIndex == UINT16_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint16_t) +
				indexOffset * sizeof(uint16_t) <=
				indexBuffer->base.size);
		}
		else if (drawIndex == UINT32_DRAW_INDEX)
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

	mesh->base.drawIndex = drawIndex;
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
