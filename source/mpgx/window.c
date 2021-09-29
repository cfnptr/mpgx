#include "mpgx/window.h"
#include "mpgx/_source/mesh.h"
#include "mpgx/_source/sampler.h"
#include "mpgx/_source/framebuffer.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "cmmt/common.h"
#include <stdio.h>

// TODO: OpenGL renderbuffer optimization

struct ImageData
{
	uint8_t* pixels;
	Vec2U size;
	uint8_t channelCount;
};

struct Window
{
	uint8_t api;
	bool useStencilBuffer;
	OnWindowUpdate onUpdate;
	void* updateArgument;
	GLFWwindow* handle;
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
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	Mesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
	Vec2U framebufferSize;
	double updateTime;
	double deltaTime;
	bool isRecording;
	bool isRendering;
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

	graphicsInitialized = true;
	return true;
}
void terminateGraphics()
{
	assert(graphicsInitialized == true);

#if MPGX_SUPPORT_VULKAN
	destroyVkDebugUtilsMessenger(
		vkInstance,
		vkDebugUtilsMessenger);
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
	uint8_t api,
	bool useStencilBuffer,
	Vec2U size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool isVisible,
	size_t bufferCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t framebufferCapacity,
	size_t shaderCapacity,
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
	assert(framebufferCapacity != 0);
	assert(shaderCapacity != 0);
	assert(pipelineCapacity != 0);
	assert(graphicsInitialized == true);
	assert(meshCapacity != 0);

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

		//glEnable(GL_FRAMEBUFFER_SRGB);
	}

	Buffer* buffers = malloc(
		sizeof(Buffer) * bufferCapacity);

	if (buffers == NULL)
	{
#if MPGX_SUPPORT_VULKAN
		destroyVkWindow(vkInstance, vkWindow);
#endif
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Image* images = malloc(
		sizeof(Image) * imageCapacity);

	if (images == NULL)
	{
		free(buffers);
#if MPGX_SUPPORT_VULKAN
		destroyVkWindow(vkInstance, vkWindow);
#endif
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
#if MPGX_SUPPORT_VULKAN
		destroyVkWindow(vkInstance, vkWindow);
#endif
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Framebuffer* framebuffers = malloc(
		sizeof(Framebuffer) * framebufferCapacity);

	if (framebuffers == NULL)
	{
		free(samplers);
		free(images);
		free(buffers);
#if MPGX_SUPPORT_VULKAN
		destroyVkWindow(vkInstance, vkWindow);
#endif
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Shader* shaders = malloc(
		sizeof(Shader) * shaderCapacity);

	if (shaders == NULL)
	{
		free(framebuffers);
		free(samplers);
		free(images);
		free(buffers);
#if MPGX_SUPPORT_VULKAN
		destroyVkWindow(vkInstance, vkWindow);
#endif
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Pipeline* pipelines = malloc(
		sizeof(Pipeline) * pipelineCapacity);

	if (pipelines == NULL)
	{
		free(shaders);
		free(framebuffers);
		free(samplers);
		free(images);
		free(buffers);
#if MPGX_SUPPORT_VULKAN
		destroyVkWindow(vkInstance, vkWindow);
#endif
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Mesh* meshes = malloc(
		sizeof(Mesh) * meshCapacity);

	if (meshes == NULL)
	{
		free(pipelines);
		free(shaders);
		free(framebuffers);
		free(samplers);
		free(images);
		free(meshes);
		free(buffers);
#if MPGX_SUPPORT_VULKAN
		destroyVkWindow(vkInstance, vkWindow);
#endif
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	window->api = api;
	window->useStencilBuffer = useStencilBuffer;
	window->onUpdate = onUpdate;
	window->updateArgument = updateArgument;
	window->handle = handle;
	window->buffers = buffers;
	window->bufferCapacity = bufferCapacity;
	window->bufferCount = 0;
	window->images = images;
	window->imageCapacity = imageCapacity;
	window->imageCount = 0;
	window->samplers = samplers;
	window->samplerCapacity = samplerCapacity;
	window->samplerCount = 0;
	window->framebuffers = framebuffers;
	window->framebufferCapacity = framebufferCapacity;
	window->framebufferCount = 0;
	window->shaders = shaders;
	window->shaderCapacity = shaderCapacity;
	window->shaderCount = 0;
	window->pipelines = pipelines;
	window->pipelineCapacity = pipelineCapacity;
	window->pipelineCount = 0;
	window->meshes = meshes;
	window->meshCapacity = meshCapacity;
	window->meshCount = 0;
	window->framebufferSize = framebufferSize;
	window->updateTime = 0.0;
	window->deltaTime = 0.0;
	window->isRecording = false;
	window->isRendering = false;
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
	size_t meshCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t framebufferCapacity,
	size_t shaderCapacity,
	size_t pipelineCapacity)
{
	assert(size.x > 0);
	assert(size.y > 0);
	assert(title != NULL);
	assert(updateFunction != NULL);
	assert(bufferCapacity != 0);
	assert(meshCapacity != 0);
	assert(imageCapacity != 0);
	assert(samplerCapacity != 0);
	assert(framebufferCapacity != 0);
	assert(shaderCapacity != 0);
	assert(pipelineCapacity != 0);
	assert(graphicsInitialized == true);

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
		framebufferCapacity,
		shaderCapacity,
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
		framebufferCapacity,
		shaderCapacity,
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
		framebufferCapacity,
		shaderCapacity,
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
	Pipeline* pipelines = window->pipelines;
	size_t pipelineCount = window->pipelineCount;
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

	uint8_t api = window->api;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		if (pipeline->vk.onHandleDestroy != NULL)
		{
			pipeline->vk.onHandleDestroy(
				window,
				pipeline->vk.handle);
		}
	}

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

		for (size_t i = 0; i < pipelineCount; i++)
			destroyVkPipeline(device, pipelines[i]);
		for (size_t i = 0; i < meshCount; i++)
			destroyVkMesh(meshes[i]);
		for (size_t i = 0; i < shaderCount; i++)
			destroyVkShader(device, shaders[i]);
		for (size_t i = 0; i < framebufferCount; i++)
			destroyVkFramebuffer(device, framebuffers[i]);
		for (size_t i = 0; i < samplerCount; i++)
			destroyVkSampler(device, samplers[i]);
		for (size_t i = 0; i < imageCount; i++)
			destroyVkImage(allocator, images[i]);
		for (size_t i = 0; i < bufferCount; i++)
			destroyVkBuffer(allocator, buffers[i]);

		destroyVkWindow(
			vkInstance,
			vkWindow);
#endif
	}
    else if (api == OPENGL_GRAPHICS_API ||
    	api == OPENGL_ES_GRAPHICS_API)
	{
		for (size_t i = 0; i < pipelineCount; i++)
			destroyGlPipeline(pipelines[i]);
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
	free(pipelines);

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
		window->framebufferCount == 0 &&
		window->shaderCount == 0 &&
		window->pipelineCount == 0 &&
		window->meshCount == 0;
}
uint8_t getWindowGraphicsAPI(Window window)
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
Vec2U getWindowFramebufferSize(Window window)
{
	assert(window != NULL);
	return window->framebufferSize;
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

	uint8_t api = window->api;

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

bool getWindowKeyboardKey(
	Window window,
	int key)
{
	assert(window != NULL);

	return glfwGetKey(
		window->handle,
		key) == GLFW_PRESS;
}
bool getWindowMouseButton(
	Window window,
	int button)
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

uint8_t getWindowCursorMode(
	Window window)
{
	assert(window != NULL);

	return glfwGetInputMode(
		window->handle,
		GLFW_CURSOR);
}
void setWindowCursorMode(
	Window window,
	uint8_t cursorMode)
{
	assert(window != NULL);
	assert(cursorMode < CURSOR_MODE_COUNT);

	int value;

	if (cursorMode == DEFAULT_CURSOR_MODE)
		value = GLFW_CURSOR_NORMAL;
	else if (cursorMode == HIDDEN_CURSOR_MODE)
		value = GLFW_CURSOR_HIDDEN;
	else if (cursorMode == LOCKED_CURSOR_MODE)
		value = GLFW_CURSOR_DISABLED;
	else
		abort();

	glfwSetInputMode(
		window->handle,
		GLFW_CURSOR,
		value);
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

	// TODO: add vsync off/on option

	while (glfwWindowShouldClose(handle) == GLFW_FALSE)
	{
		glfwPollEvents();

		double time = glfwGetTime();
		window->deltaTime = time - window->updateTime;
		window->updateTime = time;
		onUpdate(updateArgument);
	}
}

#if MPGX_SUPPORT_VULKAN
static void onVkResize(Window window)
{
	Pipeline* pipelines = window->pipelines;
	size_t pipelineCount = window->pipelineCount;
	VkWindow vkWindow = window->vkWindow;
	VkDevice device = vkWindow->device;
	VkRenderPass renderPass = vkWindow->swapchain->renderPass;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];
		VkPipelineCreateInfo createInfo;

		if (pipeline->vk.onHandleResize != NULL)
		{
			pipeline->vk.onHandleResize(
				pipeline,
				&createInfo);
		}

		vkDestroyPipeline(
			device,
			pipeline->vk.vkHandle,
			NULL);

		VkPipeline vkHandle = createVkPipelineHandle(
			pipeline->vk.cache,
			pipeline->vk.layout,
			device,
			renderPass,
			&createInfo,
			window,
			pipeline->vk.shaders,
			pipeline->vk.shaderCount,
			pipeline->vk.state);

		if (vkHandle == NULL)
			abort();

		pipeline->vk.vkHandle = vkHandle;
	}
}
#endif

static void onGlResize(Window window)
{
	Pipeline* pipelines = window->pipelines;
	size_t pipelineCount = window->pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		if (pipeline->gl.onHandleResize != NULL)
		{
			pipeline->gl.onHandleResize(
				pipeline,
				NULL);
		}
	}
}

bool beginWindowRecord(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == false);
	assert(window->isRendering == false);

	int width, height;

	glfwGetFramebufferSize(
		window->handle,
		&width,
		&height);

	if (width <= 0 || height <= 0)
		return false;

	Vec2U framebufferSize =
		vec2U(width, height);
	Vec2U currentFramebufferSize =
		window->framebufferSize;

	bool isResized = false;

	if (framebufferSize.x != currentFramebufferSize.x ||
		framebufferSize.y != currentFramebufferSize.y)
	{
		window->framebufferSize = framebufferSize;
		isResized = true;
	}

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bool result = beginVkWindowRecord(
			window,
			window->vkWindow,
			isResized,
			onVkResize);

		if (result == false)
			return false;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		if (isResized == true)
			onGlResize(window);
	}
	else
	{
		abort();
	}

	window->isRecording = true;
	return true;
}

inline static void endGlWindowRecord(GLFWwindow* window)
{
	glfwSwapBuffers(window);
}
void endWindowRecord(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == true);
	assert(window->isRendering == false);

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bool result = endVkWindowRecord(
			window,
			window->vkWindow,
			onVkResize);

		if (result == false)
			abort();
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		endGlWindowRecord(window->handle);
	}
	else
	{
		abort();
	}

	window->isRecording = false;
}

inline static void beginGlWindowRender(
	bool useStencilBuffer,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil)
{
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		GL_ZERO);

	glClearColor(
		clearColor.x,
		clearColor.y,
		clearColor.z,
		clearColor.w);
	glColorMask(
		GL_TRUE, GL_TRUE,
		GL_TRUE, GL_TRUE);

	glClearDepth(clearDepth);
	glDepthMask(GL_TRUE);

	GLbitfield clearMask =
		GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT;

	if (useStencilBuffer == true)
	{
		glClearStencil((GLint)clearStencil);
		glStencilMask(UINT32_MAX);
		clearMask |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(clearMask);
	assertOpenGL();
}
void beginWindowRender(
	Window window,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil)
{
	assert(window != NULL);
	assert(
		clearColor.x >= 0.0f &&
		clearColor.y >= 0.0f &&
		clearColor.z >= 0.0f &&
		clearColor.w >= 0.0f);
	assert(
		clearColor.x <= 1.0f &&
		clearColor.y <= 1.0f &&
		clearColor.z <= 1.0f &&
		clearColor.w <= 1.0f);
	assert(
		clearDepth >= 0.0f &&
		clearDepth <= 1.0f);
	assert(window->isRecording == true);
	assert(window->isRendering == false);

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bool result = beginVkWindowRender(
			window,
			window->vkWindow,
			clearColor,
			clearDepth,
			clearStencil);

		if (result == false)
			abort();
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		beginGlWindowRender(
			window->useStencilBuffer,
			clearColor,
			clearDepth,
			clearStencil);
	}
	else
	{
		abort();
	}

	window->isRendering = true;
}
void endWindowRender(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == true);
	assert(window->isRendering == true);

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		endVkWindowRender(window->vkWindow);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
	}
	else
	{
		abort();
	}

	window->isRendering = false;
}

Buffer createBuffer(
	Window window,
	uint8_t type,
	const void* data,
	size_t size,
	bool isConstant)
{
	assert(window != NULL);
	assert(type < BUFFER_TYPE_COUNT);
	assert(size != 0);
	assert(window->isRecording == false);

	uint8_t api = window->api;

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

	assert(buffer->vk.window->isRecording == false);

	Window window = buffer->vk.window;
	Buffer* buffers = window->buffers;
	size_t bufferCount = window->bufferCount;

	for (size_t i = 0; i < bufferCount; i++)
	{
		if (buffer != buffers[i])
			continue;

		for (size_t j = i + 1; j < bufferCount; j++)
			buffers[j - 1] = buffers[j];

		uint8_t api = window->api;

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

		window->bufferCount--;
		return;
	}

	abort();
}

Window getBufferWindow(Buffer buffer)
{
	assert(buffer != NULL);
	return buffer->vk.window;
}
uint8_t getBufferType(Buffer buffer)
{
	assert(buffer != NULL);
	return buffer->vk.type;
}
size_t getBufferSize(Buffer buffer)
{
	assert(buffer != NULL);
	return buffer->vk.size;
}
bool isBufferConstant(Buffer buffer)
{
	assert(buffer != NULL);
	return buffer->vk.isConstant;
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
	assert(buffer->vk.isConstant == false);
	assert(size + offset <= buffer->vk.size);

	uint8_t api = buffer->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		bool result = setVkBufferData(
			buffer->vk.window->vkWindow->allocator,
			buffer->vk.allocation,
			data,
			size,
			offset);

		if (result == false)
			abort();
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
	uint8_t type,
	uint8_t format,
	Vec3U size,
	const void** data,
	uint8_t levelCount)
{
	assert(window != NULL);
	assert(type < IMAGE_TYPE_COUNT);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(data != NULL);
	assert(levelCount >= 0);
	assert(levelCount <= getImageLevelCount(size));
	assert(window->isRecording == false);

	uint8_t api = window->api;

	Image image;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		image = createVkImage(
			window->vkWindow->allocator,
			0,
			VK_FORMAT_UNDEFINED,
			window,
			type,
			format,
			size);
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
			size,
			data,
			levelCount);
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
				destroyVkImage(
					window->vkWindow->allocator,
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
	uint8_t format,
	const char* filePath,
	bool generateMipmap)
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
		vec3U(width, height, 1),
		(const void**)&pixels,
		generateMipmap ? 0 : 1);

	stbi_image_free(pixels);
	return image;
}
void destroyImage(Image image)
{
	if (image == NULL)
		return;

	assert(image->vk.window->isRecording == false);

	Window window = image->vk.window;
	Image* images = window->images;
	size_t imageCount = window->imageCount;

	for (size_t i = 0; i < imageCount; i++)
	{
		if (image != images[i])
			continue;

		for (size_t j = i + 1; j < imageCount; j++)
			images[j - 1] = images[j];

		uint8_t api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow = window->vkWindow;

			VkResult result = vkDeviceWaitIdle(
				vkWindow->device);

			if (result != VK_SUCCESS)
				abort();

			destroyVkImage(
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
	assert(size.x + offset.x <= image->vk.size.x);
	assert(size.y + offset.y <= image->vk.size.y);
	assert(size.z + offset.z <= image->vk.size.z);
	assert(image->vk.window->isRecording == false);

	// TODO: check for static image in Vulkan API

	uint8_t api = image->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		setVkImageData(
			image,
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
	return image->vk.window;
}
uint8_t getImageType(Image image)
{
	assert(image != NULL);
	return image->vk.type;
}
uint8_t getImageFormat(Image image)
{
	assert(image != NULL);
	return image->vk.format;
}
Vec3U getImageSize(Image image)
{
	assert(image != NULL);
	return image->vk.size;
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
	uint8_t minImageFilter,
	uint8_t magImageFilter,
	uint8_t minMipmapFilter,
	bool useMipmapping,
	uint8_t imageWrapX,
	uint8_t imageWrapY,
	uint8_t imageWrapZ,
	uint8_t compareOperation,
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
	assert(compareOperation < COMPARE_OPERATOR_COUNT);
	assert(window->isRecording == false);

	uint8_t api = window->api;

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
			compareOperation,
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
			compareOperation,
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

	assert(sampler->vk.window->isRecording == false);

	Window window = sampler->vk.window;
	Sampler* samplers = window->samplers;
	size_t samplerCount = window->samplerCount;

	for (size_t i = 0; i < samplerCount; i++)
	{
		if (sampler != samplers[i])
			continue;

		for (size_t j = i + 1; j < samplerCount; j++)
			samplers[j - 1] = samplers[j];

		uint8_t api = window->api;

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

		window->samplerCount--;
		return;
	}

	abort();
}

Window getSamplerWindow(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.window;
}
uint8_t getSamplerMinImageFilter(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.minImageFilter;
}
uint8_t getSamplerMagImageFilter(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.magImageFilter;
}
uint8_t getSamplerMinMipmapFilter(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.minMipmapFilter;
}
bool isSamplerUseMipmapping(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.useMipmapping;
}
uint8_t getSamplerImageWrapX(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.imageWrapX;
}
uint8_t getSamplerImageWrapY(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.imageWrapY;
}
uint8_t getSamplerImageWrapZ(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.imageWrapZ;
}
uint8_t getSamplerCompareOperation(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.compareOperation;
}
bool isSamplerUseCompare(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.useCompare;
}
Vec2F getSamplerMipmapLodRange(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.mipmapLodRange;
}
float getSamplerMipmapLodBias(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.mipmapLodBias;
}

Framebuffer createFramebuffer(
	Window window,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	assert(window != NULL);
	assert((colorAttachments == NULL &&
		colorAttachmentCount == 0) ||
		(colorAttachments != NULL &&
		colorAttachmentCount != 0));
	assert(colorAttachments != NULL ||
		depthStencilAttachment != NULL);
	assert(getImageWindow(depthStencilAttachment) == window);
	assert(window->isRecording == false);

	uint8_t api = window->api;

	Framebuffer framebuffer;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = window->vkWindow;

		framebuffer = createVkFramebuffer(
			vkWindow->device,
			window,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		framebuffer = createGlFramebuffer(
			window,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);
	}
	else
	{
		abort();
	}

	if (framebuffer == NULL)
		return NULL;

	size_t count = window->framebufferCount;

	if (count == window->framebufferCapacity)
	{
		size_t capacity = window->framebufferCapacity * 2;

		Framebuffer* framebuffers = realloc(
			window->framebuffers,
			sizeof(Framebuffer) * capacity);

		if (framebuffers == NULL)
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

		window->framebuffers = framebuffers;
		window->framebufferCapacity = capacity;
	}

	window->framebuffers[count] = framebuffer;
	window->framebufferCount = count + 1;
	return framebuffer;
}
void destroyFramebuffer(Framebuffer framebuffer)
{
	if (framebuffer == NULL)
		return;

	assert(framebuffer->vk.window->isRecording == false);

	Window window = framebuffer->vk.window;
	Framebuffer* framebuffers = window->framebuffers;
	size_t framebufferCount = window->framebufferCount;

	for (size_t i = 0; i < framebufferCount; i++)
	{
		if (framebuffer != framebuffers[i])
			continue;

		for (size_t j = i + 1; j < framebufferCount; j++)
			framebuffers[j - 1] = framebuffers[j];

		uint8_t api = window->api;

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

		window->framebufferCount--;
		return;
	}

	abort();
}

Image* getFramebufferColorAttachments(
	Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	return framebuffer->vk.colorAttachments;
}
size_t getFramebufferColorAttachmentCount(
	Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	return framebuffer->vk.colorAttachmentCount;
}
Image getFramebufferDepthStencilAttachment(
	Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	return framebuffer->vk.depthStencilAttachment;
}

void beginFramebufferRender(Framebuffer framebuffer)
{
	assert(framebuffer != NULL);
	assert(framebuffer->vk.window->isRecording == true);
	assert(framebuffer->vk.window->isRendering == false);

	Window window = framebuffer->vk.window;
	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		beginVkFramebufferRender(framebuffer);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		beginGlFramebufferRender(
			framebuffer->gl.handle);
	}
	else
	{
		abort();
	}

	window->isRendering = true;
}

void endFramebufferRender(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == true);
	assert(window->isRendering == true);

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		endVkFramebufferRender(window);
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

	window->isRendering = false;
}

void clearFramebuffer(
	Window window,
	bool clearColorBuffer,
	bool clearDepthBuffer,
	bool clearStencilBuffer,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil)
{
	assert(window != NULL);
	assert(
		clearColorBuffer == true ||
		clearDepthBuffer == true ||
		clearStencilBuffer == true);
	assert(window->useStencilBuffer == true ||
		clearStencilBuffer == false);
	assert(
		clearColor.x >= 0.0f &&
		clearColor.y >= 0.0f &&
		clearColor.z >= 0.0f &&
		clearColor.w >= 0.0f);
	assert(
		clearColor.x <= 1.0f &&
		clearColor.y <= 1.0f &&
		clearColor.z <= 1.0f &&
		clearColor.w <= 1.0f);
	assert(
		clearDepth >= 0.0f &&
		clearDepth <= 1.0f);
	assert(window->isRecording == true);

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		clearVkFramebuffer(
			clearColorBuffer,
			clearDepthBuffer,
			clearStencilBuffer,
			clearColor,
			clearDepth,
			clearStencil);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
			 api == OPENGL_ES_GRAPHICS_API)
	{
		clearGlFramebuffer(
			clearColorBuffer,
			clearDepthBuffer,
			clearStencilBuffer,
			clearColor,
			clearDepth,
			clearStencil);
	}
	else
	{
		abort();
	}
}

Shader createShader(
	Window window,
	uint8_t type,
	const void* code,
	size_t size)
{
	assert(window != NULL);
	assert(type < SHADER_TYPE_COUNT);
	assert(code != NULL);
	assert(window->isRecording == false);

	uint8_t api = window->api;

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
	uint8_t type,
	const char* filePath)
{
	assert(window != NULL);
	assert(filePath != NULL);

	FILE* file = fopen(
		filePath,
		"rb");

	if (file == NULL)
		return NULL;

	int seekResult = fseek(
		file,
		0,
		SEEK_END);

	if (seekResult != 0)
	{
		fclose(file);
		return NULL;
	}

	size_t fileSize = ftell(file);

	seekResult = fseek(
		file,
		0,
		SEEK_SET);

	if (seekResult != 0)
	{
		fclose(file);
		return NULL;
	}

	uint8_t api = window->api;

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

	assert(shader->vk.window->isRecording == false);

	Window window = shader->vk.window;
	Shader* shaders = window->shaders;
	size_t shaderCount = window->shaderCount;

	for (size_t i = 0; i < shaderCount; i++)
	{
		if (shader != shaders[i])
			continue;

		for (size_t j = i + 1; j < shaderCount; j++)
			shaders[j - 1] = shaders[j];

		uint8_t api = window->api;

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

		window->shaderCount--;
		return;
	}

	abort();
}

Window getShaderWindow(Shader shader)
{
	assert(shader != NULL);
	return shader->vk.window;
}
uint8_t getShaderType(Shader shader)
{
	assert(shader != NULL);
	return shader->vk.type;
}

Pipeline createPipeline(
	Window window,
	const char* name,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	OnPipelineHandleDestroy onHandleDestroy,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineHandleResize onHandleResize,
	void* handle,
	void* createInfo)
{
	assert(window != NULL);
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
	assert(window->isRecording == false);

	uint8_t api = window->api;

	Pipeline pipeline;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		assert(createInfo != NULL);
		VkWindow vkWindow = window->vkWindow;

		pipeline = createVkPipeline(
			vkWindow->device,
			vkWindow->swapchain->renderPass,
			createInfo,
			window,
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
		assert(state->restartPrimitive == false);
		assert(state->discardRasterizer == false);

		pipeline = createGlPipeline(
			window,
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

	size_t count = window->pipelineCount;

	if (count == window->pipelineCapacity)
	{
		size_t capacity = window->pipelineCapacity * 2;

		Pipeline* pipelines = realloc(
			window->pipelines,
			sizeof(Pipeline) * capacity);

		if (pipelines == NULL)
		{
			free(pipeline);
			return NULL;
		}

		window->pipelines = pipelines;
		window->pipelineCapacity = capacity;
	}

	window->pipelines[count] = pipeline;
	window->pipelineCount = count + 1;
	return pipeline;
}
void destroyPipeline(
	Pipeline pipeline,
	bool destroyShaders)
{
	if (pipeline == NULL)
		return;

	assert(pipeline->vk.window->isRecording == false);

	Window window = pipeline->vk.window;
	Pipeline* pipelines = window->pipelines;
	size_t pipelineCount = window->pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		if (pipeline != pipelines[i])
			continue;

		for (size_t j = i + 1; j < pipelineCount; j++)
			pipelines[j - 1] = pipelines[j];

		if (pipeline->vk.onHandleDestroy != NULL)
		{
			pipeline->vk.onHandleDestroy(
				window,
				pipeline->vk.handle);
		}

		if (destroyShaders == true)
		{
			Shader* shaders = pipeline->vk.shaders;
			uint8_t shaderCount = pipeline->vk.shaderCount;

			for (uint8_t j = 0; j < shaderCount; j++)
				destroyShader(shaders[j]);
		}

		uint8_t api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			VkWindow vkWindow =
				pipeline->vk.window->vkWindow;
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

		window->pipelineCount--;
		return;
	}

	abort();
}

Window getPipelineWindow(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.window;
}
const char* getPipelineName(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.name;
}
Shader* getPipelineShaders(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.shaders;
}
uint8_t getPipelineShaderCount(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.shaderCount;
}
const PipelineState* getPipelineState(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return &pipeline->vk.state;
}
OnPipelineHandleDestroy getPipelineOnHandleDestroy(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.onHandleDestroy;
}
OnPipelineHandleBind getPipelineOnHandleBind(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.onHandleBind;
}
OnPipelineUniformsSet getPipelineOnUniformsSet(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.onUniformsSet;
}
void* getPipelineHandle(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->vk.handle;
}

void bindPipeline(Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->vk.window->isRecording == true);

	uint8_t api = pipeline->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow =
			pipeline->vk.window->vkWindow;
		bindVkPipeline(
			vkWindow->currenCommandBuffer,
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
	uint8_t drawIndex,
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
		assert(vertexBuffer->vk.window == window);
		assert(vertexBuffer->vk.type == VERTEX_BUFFER_TYPE);
	}
	if (indexBuffer != NULL)
	{
		assert(indexBuffer->vk.window == window);
		assert(indexBuffer->vk.type == INDEX_BUFFER_TYPE);

		if (drawIndex == UINT16_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint16_t) +
				   indexOffset * sizeof(uint16_t) <=
				   indexBuffer->vk.size);
		}
		else if (drawIndex == UINT32_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint32_t) +
				   indexOffset * sizeof(uint32_t) <=
				   indexBuffer->vk.size);
		}
		else
		{
			abort();
		}
	}
#endif

	uint8_t api = window->api;

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

	assert(mesh->vk.window->isRecording == false);

	Window window = mesh->vk.window;
	Mesh* meshes = window->meshes;
	size_t meshCount = window->meshCount;

	for (size_t i = 0; i < meshCount; i++)
	{
		if (mesh != meshes[i])
			continue;

		for (size_t j = i + 1; j < meshCount; j++)
			meshes[j - 1] = meshes[j];

		if (destroyBuffers == true)
		{
			destroyBuffer(mesh->vk.vertexBuffer);
			destroyBuffer(mesh->vk.indexBuffer);
		}

		uint8_t api = window->api;

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

		window->meshCount--;
		return;
	}

	abort();
}

Window getMeshWindow(Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->vk.window;
}
uint8_t getMeshDrawIndex(Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->vk.drawIndex;
}

size_t getMeshIndexCount(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->vk.indexCount;
}
void setMeshIndexCount(
	Mesh mesh,
	size_t indexCount)
{
	assert(mesh != NULL);
	assert(mesh->vk.window->isRecording == false);

#ifndef NDEBUG
	if (mesh->vk.indexBuffer != NULL)
	{
		if (mesh->vk.drawIndex == UINT16_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint16_t) +
				   mesh->vk.indexOffset * sizeof(uint16_t) <=
				   mesh->vk.indexBuffer->vk.size);
		}
		else if (mesh->vk.drawIndex == UINT32_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint32_t) +
				   mesh->vk.indexOffset * sizeof(uint32_t) <=
				   mesh->vk.indexBuffer->vk.size);
		}
		else
		{
			abort();
		}
	}
#endif

	mesh->vk.indexCount = indexCount;
}

size_t getMeshIndexOffset(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->vk.indexOffset;
}
void setMeshIndexOffset(
	Mesh mesh,
	size_t indexOffset)
{
	assert(mesh != NULL);
	assert(mesh->vk.window->isRecording == false);

#ifndef NDEBUG
	if (mesh->vk.indexBuffer != NULL)
	{
		if (mesh->vk.drawIndex == UINT16_DRAW_INDEX)
		{
			assert(mesh->vk.indexCount * sizeof(uint16_t) +
				   indexOffset * sizeof(uint16_t) <=
				   mesh->vk.indexBuffer->vk.size);
		}
		else if (mesh->vk.drawIndex == UINT32_DRAW_INDEX)
		{
			assert(mesh->vk.indexCount * sizeof(uint32_t) +
				   indexOffset * sizeof(uint32_t) <=
				   mesh->vk.indexBuffer->vk.size);
		}
		else
		{
			abort();
		}
	}
#endif

	mesh->vk.indexOffset = indexOffset;
}

Buffer getMeshVertexBuffer(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->vk.vertexBuffer;
}
void setMeshVertexBuffer(
	Mesh mesh,
	Buffer vertexBuffer)
{
	assert(mesh != NULL);
	assert(mesh->vk.window->isRecording == false);

#ifndef NDEBUG
	if (vertexBuffer != NULL)
	{
		assert(mesh->vk.window == vertexBuffer->vk.window);
		assert(vertexBuffer->vk.type == VERTEX_BUFFER_TYPE);
	}
#endif

	mesh->vk.vertexBuffer = vertexBuffer;
}

Buffer getMeshIndexBuffer(
	Mesh mesh)
{
	assert(mesh != NULL);
	return mesh->vk.indexBuffer;
}
void setMeshIndexBuffer(
	Mesh mesh,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer indexBuffer)
{
	assert(mesh != NULL);
	assert(drawIndex < DRAW_INDEX_COUNT);
	assert(mesh->vk.window->isRecording == false);

#ifndef NDEBUG
	if (indexBuffer != NULL)
	{
		assert(mesh->vk.window == indexBuffer->vk.window);
		assert(indexBuffer->vk.type == INDEX_BUFFER_TYPE);

		if (drawIndex == UINT16_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint16_t) +
				   indexOffset * sizeof(uint16_t) <=
				   indexBuffer->vk.size);
		}
		else if (drawIndex == UINT32_DRAW_INDEX)
		{
			assert(indexCount * sizeof(uint32_t) +
				   indexOffset * sizeof(uint32_t) <=
				   indexBuffer->vk.size);
		}
		else
		{
			abort();
		}
	}
#endif

	mesh->vk.drawIndex = drawIndex;
	mesh->vk.indexCount = indexCount;
	mesh->vk.indexOffset = indexOffset;
	mesh->vk.indexBuffer = indexBuffer;
}

size_t drawMesh(
	Mesh mesh,
	Pipeline pipeline)
{
	assert(mesh != NULL);
	assert(pipeline != NULL);
	assert(mesh->vk.window == pipeline->vk.window);
	assert(mesh->vk.window->isRecording == true);

	if (mesh->vk.vertexBuffer == NULL ||
		mesh->vk.indexBuffer == NULL ||
		mesh->vk.indexCount == 0)
	{
		return 0;
	}

	uint8_t api = pipeline->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		if (pipeline->vk.onUniformsSet != NULL)
			pipeline->vk.onUniformsSet(pipeline);

		VkWindow vkWindow =
			mesh->vk.window->vkWindow;
		drawVkMesh(
			vkWindow->currenCommandBuffer,
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

	return mesh->vk.indexCount;
}
