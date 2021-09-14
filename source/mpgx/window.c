#include "mpgx/window.h"
#include "mpgx/opengl.h"

#include "mpgx/_source/buffer.h"
#include "mpgx/_source/mesh.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"
#include "mpgx/_source/framebuffer.h"
#include "mpgx/_source/shader.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "cmmt/common.h"
#include <stdio.h>

struct Pipeline
{
	Window window;
	const char* name;
	uint8_t drawMode;
	OnPipelineHandleDestroy onHandleDestroy;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	void* handle;
};
struct ImageData
{
	uint8_t* pixels;
	Vec2U size;
	uint8_t channelCount;
};

typedef struct _VkWindow
{
	uint8_t api;
	uint32_t maxImageSize;
	OnWindowUpdate onUpdate;
	void* updateArgument;
	GLFWwindow* handle;
	Buffer* buffers;
	size_t bufferCapacity;
	size_t bufferCount;
	Mesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
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
	double updateTime;
	double deltaTime;
	bool isRecording;
	bool isRendering;
#if MPGX_VULKAN_SUPPORT
	VkInstance vkInstance;
#endif
} _VkWindow;
typedef struct _GlWindow
{
	uint8_t api;
	uint32_t maxImageSize;
	OnWindowUpdate onUpdate;
	void* updateArgument;
	GLFWwindow* handle;
	Buffer* buffers;
	size_t bufferCapacity;
	size_t bufferCount;
	Mesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
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
	double updateTime;
	double deltaTime;
	bool isRecording;
	bool isRendering;
} _GlWindow;
union Window
{
	_VkWindow vk;
	_GlWindow gl;
};

static bool graphicsInitialized = false;
static FT_Library ftLibrary = NULL;
static Window currentWindow = NULL;

static void glfwErrorCallback(
	int error,
	const char* description)
{
	fprintf(stderr,
		"GLFW error: %d, %s\n",
		error,
		description);
	abort();
}

bool initializeGraphics()
{
	if (graphicsInitialized == true)
		return false;

	if(glfwInit() == GLFW_FALSE)
		return false;

	glfwSetErrorCallback(glfwErrorCallback);

	if (FT_Init_FreeType(&ftLibrary) != 0)
		return false;

	graphicsInitialized = true;
	return true;
}
void terminateGraphics()
{
	if (graphicsInitialized == false)
		return;

	if (FT_Done_FreeType(ftLibrary) != 0)
		abort();

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

#if MPGX_VULKAN_SUPPORT
inline static VkInstance createVkInstance()
{
	VkInstanceCreateInfo createInfo = {
		// TODO:
	};

	VkInstance vkInstance;

	VkResult result = vkCreateInstance(
		&createInfo,
		NULL,
		&vkInstance);

	if (result != VK_SUCCESS)
		return NULL;

	return vkInstance;
}
inline static void destroyVkInstace(VkInstance vkInstance)
{
	if (vkInstance == NULL)
		return;

	vkDestroyInstance(
		vkInstance,
		NULL);
}
#endif

// TODO: createVkWindow, createGlWindow...
// #if MPGX_VULKAN_SUPPORT
//	window->vk.vkInstance = vkInstance;
//#endif
Window createWindow(
	uint8_t api,
	Vec2U size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool isVisible,
	size_t bufferCapacity,
	size_t meshCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t framebufferCapacity,
	size_t shaderCapacity,
	size_t pipelineCapacity)
{
	assert(api < GRAPHICS_API_COUNT);
	assert(size.x != 0);
	assert(size.y != 0);
	assert(title != NULL);
	assert(onUpdate != NULL);
	assert(bufferCapacity != 0);
	assert(meshCapacity != 0);
	assert(imageCapacity != 0);
	assert(samplerCapacity != 0);
	assert(framebufferCapacity != 0);
	assert(shaderCapacity != 0);
	assert(pipelineCapacity != 0);
	assert(graphicsInitialized == true);

	Window window = malloc(
		sizeof(union Window));

	if (window == NULL)
		return NULL;

	glfwDefaultWindowHints();

	glfwWindowHint(
		GLFW_VISIBLE,
		isVisible ? GLFW_TRUE : GLFW_FALSE);

	if (api == VULKAN_GRAPHICS_API)
	{
		if (glfwVulkanSupported() == GLFW_FALSE)
		{
			free(window);
			return NULL;
		}

		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_NO_API);

		free(window);
		return NULL;
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
		1,
		1,
		GLFW_DONT_CARE,
		GLFW_DONT_CARE);

	if (glfwRawMouseMotionSupported() == GLFW_TRUE)
	{
		glfwSetInputMode(
			handle,
			GLFW_RAW_MOUSE_MOTION,
			GLFW_TRUE);
	}

	uint32_t maxImageSize;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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

		GLint glMaxImageSize;

		glGetIntegerv(
			GL_MAX_TEXTURE_SIZE,
			&glMaxImageSize);
		assertOpenGL();

		maxImageSize = glMaxImageSize;
	}

	Buffer* buffers = malloc(
		sizeof(Buffer) * bufferCapacity);

	if (buffers == NULL)
	{
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Mesh* meshes = malloc(
		sizeof(Mesh) * meshCapacity);

	if (meshes == NULL)
	{
		free(buffers);
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Image* images = malloc(
		sizeof(Image) * imageCapacity);

	if (images == NULL)
	{
		free(meshes);
		free(buffers);
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	Sampler* samplers = malloc(
		sizeof(Sampler) * samplerCapacity);

	if (samplers == NULL)
	{
		free(images);
		free(meshes);
		free(buffers);
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
		free(meshes);
		free(buffers);
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
		free(meshes);
		free(buffers);
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
		free(meshes);
		free(buffers);
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

#if MPGX_VULKAN_SUPPORT
	VkInstance vkInstance = createVkInstance();

	if (vkInstance == NULL)
	{
		free(pipelines);
		free(shaders);
		free(framebuffers);
		free(samplers);
		free(images);
		free(meshes);
		free(buffers);
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}
#endif

	window->vk.api = api;
	window->vk.maxImageSize = maxImageSize;
	window->vk.onUpdate = onUpdate;
	window->vk.updateArgument = updateArgument;
	window->vk.handle = handle;
	window->vk.buffers = buffers;
	window->vk.bufferCapacity = bufferCapacity;
	window->vk.bufferCount = 0;
	window->vk.meshes = meshes;
	window->vk.meshCapacity = meshCapacity;
	window->vk.meshCount = 0;
	window->vk.images = images;
	window->vk.imageCapacity = imageCapacity;
	window->vk.imageCount = 0;
	window->vk.samplers = samplers;
	window->vk.samplerCapacity = samplerCapacity;
	window->vk.samplerCount = 0;
	window->vk.framebuffers = framebuffers;
	window->vk.framebufferCapacity = framebufferCapacity;
	window->vk.framebufferCount = 0;
	window->vk.shaders = shaders;
	window->vk.shaderCapacity = shaderCapacity;
	window->vk.shaderCount = 0;
	window->vk.pipelines = pipelines;
	window->vk.pipelineCapacity = pipelineCapacity;
	window->vk.pipelineCount = 0;
	window->vk.updateTime = 0.0;
	window->vk.deltaTime = 0.0;
	window->vk.isRecording = false;
	window->vk.isRendering = false;

	currentWindow = window;
	return window;
}
Window createAnyWindow(
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
		size,
		title,
		updateFunction,
		updateArgument,
		visible,
		bufferCapacity,
		meshCapacity,
		imageCapacity,
		samplerCapacity,
		framebufferCapacity,
		shaderCapacity,
		pipelineCapacity);

	if (window != NULL)
		return window;

	window = createWindow(
		OPENGL_GRAPHICS_API,
		size,
		title,
		updateFunction,
		updateArgument,
		visible,
		bufferCapacity,
		meshCapacity,
		imageCapacity,
		samplerCapacity,
		framebufferCapacity,
		shaderCapacity,
		pipelineCapacity);

	if (window != NULL)
		return window;

	window = createWindow(
		OPENGL_ES_GRAPHICS_API,
		size,
		title,
		updateFunction,
		updateArgument,
		visible,
		bufferCapacity,
		meshCapacity,
		imageCapacity,
		samplerCapacity,
		framebufferCapacity,
		shaderCapacity,
		pipelineCapacity);

	return window;
}
void destroyWindow(Window window)
{
	if (window == NULL)
        return;

	Pipeline* pipelines = window->vk.pipelines;
	size_t pipelineCount = window->vk.pipelineCount;
	Shader* shaders = window->vk.shaders;
	size_t shaderCount = window->vk.shaderCount;
	Framebuffer* framebuffers = window->vk.framebuffers;
	size_t framebufferCount = window->vk.framebufferCount;
	Sampler* samplers = window->vk.samplers;
	size_t samplerCount = window->vk.samplerCount;
	Image* images = window->vk.images;
	size_t imageCount = window->vk.imageCount;
	Mesh* meshes = window->vk.meshes;
	size_t meshCount = window->vk.meshCount;
	Buffer* buffers = window->vk.buffers;
	size_t bufferCount = window->vk.bufferCount;

	uint8_t api = window->vk.api;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		pipeline->onHandleDestroy(
			window,
			pipeline->handle);
	}

    if (api == VULKAN_GRAPHICS_API)
	{
		for (size_t i = 0; i < shaderCount; i++)
			destroyVkShader(shaders[i]);
		for (size_t i = 0; i < framebufferCount; i++)
			destroyVkFramebuffer(framebuffers[i]);
		for (size_t i = 0; i < samplerCount; i++)
			destroyVkSampler(samplers[i]);
		for (size_t i = 0; i < imageCount; i++)
			destroyVkImage(images[i]);
		for (size_t i = 0; i < meshCount; i++)
			destroyVkMesh(meshes[i]);
		for (size_t i = 0; i < bufferCount; i++)
			destroyVkBuffer(buffers[i]);
	}
    else if (api == OPENGL_GRAPHICS_API ||
    	api == OPENGL_ES_GRAPHICS_API)
	{
		for (size_t i = 0; i < shaderCount; i++)
			destroyGlShader(shaders[i]);
		for (size_t i = 0; i < framebufferCount; i++)
			destroyGlFramebuffer(framebuffers[i]);
		for (size_t i = 0; i < samplerCount; i++)
			destroyGlSampler(samplers[i]);
		for (size_t i = 0; i < imageCount; i++)
			destroyGlImage(images[i]);
		for (size_t i = 0; i < meshCount; i++)
			destroyGlMesh(meshes[i]);
		for (size_t i = 0; i < bufferCount; i++)
			destroyGlBuffer(buffers[i]);
	}
    else
	{
    	abort();
	}

#if MPGX_VULKAN_SUPPORT
	destroyVkInstace(window->vk.vkInstance);
#endif

	free(buffers);
	free(meshes);
	free(images);
	free(framebuffers);
	free(shaders);
	free(pipelines);

	glfwDestroyWindow(window->vk.handle);
	free(window);
}

bool isWindowEmpty(Window window)
{
	assert(window != NULL);

	return
		window->vk.bufferCount == 0 &&
		window->vk.meshCount == 0 &&
		window->vk.imageCount == 0 &&
		window->vk.samplerCount == 0 &&
		window->vk.framebufferCount == 0 &&
		window->vk.shaderCount == 0 &&
		window->vk.pipelineCount == 0;
}
uint8_t getWindowGraphicsAPI(Window window)
{
	assert(window != NULL);
	return window->vk.api;
}
OnWindowUpdate getWindowOnUpdate(Window window)
{
	assert(window != NULL);
	return window->vk.onUpdate;
}
void* getWindowUpdateArgument(Window window)
{
	assert(window != NULL);
	return window->vk.updateArgument;
}
uint32_t getWindowMaxImageSize(Window window)
{
	assert(window != NULL);
	return window->vk.maxImageSize;
}
double getWindowUpdateTime(Window window)
{
	assert(window != NULL);
	return window->vk.updateTime;
}
double getWindowDeltaTime(Window window)
{
	assert(window != NULL);
	return window->vk.deltaTime;
}
Vec2F getWindowContentScale(Window window)
{
	assert(window != NULL);

	Vec2F scale;

	glfwGetWindowContentScale(
		window->vk.handle,
		&scale.x,
		&scale.y);

	return scale;
}
Vec2U getWindowFramebufferSize(Window window)
{
	assert(window != NULL);

	int width, height;

	glfwGetFramebufferSize(
		window->vk.handle,
		&width,
		&height);

	return vec2U(width, height);
}
const char* getWindowClipboard(Window window)
{
	assert(window != NULL);
	return glfwGetClipboardString(window->vk.handle);
}

inline static const char* getVkWindowGpuName(Window window)
{
	return NULL;
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

	uint8_t api = window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		return getVkWindowGpuName(window);
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

inline static const char* getVkWindowGpuVendor(Window window)
{
	return NULL;
}
inline static const char* getGlWindowGpuVendor()
{
	const char* vendor = (const char*)
		glGetString(GL_VENDOR);
	assertOpenGL();
	return vendor;
}
const char* getWindowGpuVendor(Window window)
{
	assert(window != NULL);

	uint8_t api = window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		return getVkWindowGpuVendor(window);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return getGlWindowGpuVendor();
	}
	else
	{
		abort();
	}
}

bool getWindowKeyboardKey(
	Window window,
	int key)
{
	assert(window != NULL);

	return glfwGetKey(
		window->vk.handle,
		key) == GLFW_PRESS;
}
bool getWindowMouseButton(
	Window window,
	int button)
{
	assert(window != NULL);

	return glfwGetMouseButton(
		window->vk.handle,
		button) == GLFW_PRESS;
}

Vec2U getWindowSize(
	Window window)
{
	assert(window != NULL);

	int width, height;

	glfwGetWindowSize(
		window->vk.handle,
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
		window->vk.handle,
		(int)size.x,
		(int)size.y);
}

Vec2I getWindowPosition(
	Window window)
{
	assert(window != NULL);

	Vec2I position;

	glfwGetWindowPos(
		window->vk.handle,
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
		window->vk.handle,
		position.x,
		position.y);
}

Vec2F getWindowCursorPosition(
	Window window)
{
	assert(window != NULL);

	double x, y;

	glfwGetCursorPos(
		window->vk.handle,
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
		window->vk.handle,
		(double)position.x,
		(double)position.y);
}

uint8_t getWindowCursorMode(
	Window window)
{
	assert(window != NULL);

	return glfwGetInputMode(
		window->vk.handle,
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
		window->vk.handle,
		GLFW_CURSOR,
		value);
}

bool isWindowFocused(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->vk.handle,
		GLFW_FOCUSED) == GLFW_TRUE;
}
bool isWindowIconified(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->vk.handle,
		GLFW_ICONIFIED) == GLFW_TRUE;
}
bool isWindowMaximized(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->vk.handle,
		GLFW_MAXIMIZED) == GLFW_TRUE;
}
bool isWindowVisible(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->vk.handle,
		GLFW_VISIBLE) == GLFW_TRUE;
}
bool isWindowHovered(Window window)
{
	assert(window != NULL);

	return glfwGetWindowAttrib(
		window->vk.handle,
		GLFW_HOVERED) == GLFW_TRUE;
}

void iconifyWindow(Window window)
{
	assert(window != NULL);
	glfwIconifyWindow(window->vk.handle);
}
void maximizeWindow(Window window)
{
	assert(window != NULL);
	glfwMaximizeWindow(window->vk.handle);
}
void restoreWindow(Window window)
{
	assert(window != NULL);
	glfwRestoreWindow(window->vk.handle);
}
void showWindow(Window window)
{
	assert(window != NULL);
	glfwShowWindow(window->vk.handle);
}
void hideWindow(Window window)
{
	assert(window != NULL);
	glfwHideWindow(window->vk.handle);
}
void focusWindow(Window window)
{
	assert(window != NULL);
	glfwFocusWindow(window->vk.handle);
}
void requestWindowAttention(Window window)
{
	assert(window != NULL);
	glfwRequestWindowAttention(window->vk.handle);
}

void makeWindowContextCurrent(Window window)
{
	assert(window != NULL);
	assert(window->vk.api == OPENGL_GRAPHICS_API ||
		window->vk.api == OPENGL_ES_GRAPHICS_API);
	assert(window->vk.isRecording == false);

	if (window != currentWindow)
	{
		glfwMakeContextCurrent(window->vk.handle);
		currentWindow = window;
	}
}
void updateWindow(Window window)
{
	assert(window != NULL);
	assert(window->vk.isRecording == false);

	GLFWwindow* handle = window->vk.handle;
	OnWindowUpdate onUpdate = window->vk.onUpdate;
	void* updateArgument = window->vk.updateArgument;

	// TODO: add vsync off/on option

	while (glfwWindowShouldClose(handle) == GLFW_FALSE)
	{
		glfwPollEvents();

		double time = glfwGetTime();
		window->vk.deltaTime = time - window->vk.updateTime;
		window->vk.updateTime = time;
		onUpdate(updateArgument);
	}
}

inline static void beginVkWindowRender(Window window)
{

}
inline static void beginGlWindowRender(Window window)
{
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		GL_ZERO);
	assertOpenGL();
}
void beginWindowRender(Window window)
{
	assert(window != NULL);
	assert(window->vk.isRecording == false);
	assert(window->vk.isRendering == false);

	uint8_t api = window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		beginVkWindowRender(window);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		beginGlWindowRender(window);
	}
	else
	{
		abort();
	}

	window->vk.isRecording = true;
}

inline static void endVkWindowRender(Window window)
{

}
inline static void endGlWindowRender(Window window)
{
	glfwSwapBuffers(window->vk.handle);
}
void endWindowRender(Window window)
{
	assert(window != NULL);
	assert(window->vk.isRecording == true);
	assert(window->vk.isRendering == false);

	uint8_t api = window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		endVkWindowRender(window);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		endGlWindowRender(window);
	}
	else
	{
		abort();
	}

	window->vk.isRecording = false;
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
	assert(window->vk.isRecording == false);

	uint8_t api = window->vk.api;

	Buffer buffer;

	if (api == VULKAN_GRAPHICS_API)
	{
		buffer = createVkBuffer(
			window,
			type,
			data,
			size,
			isConstant);
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

	size_t count = window->vk.bufferCount;

	if (count == window->vk.bufferCapacity)
	{
		size_t capacity = window->vk.bufferCapacity * 2;

		Buffer* buffers = realloc(
			window->vk.buffers,
			sizeof(Buffer) * capacity);

		if (buffers == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
				destroyVkBuffer(buffer);
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

		window->vk.buffers = buffers;
		window->vk.bufferCapacity = capacity;
	}

	window->vk.buffers[count] = buffer;
	window->vk.bufferCount = count + 1;
	return buffer;
}
void destroyBuffer(Buffer buffer)
{
	if (buffer == NULL)
		return;

	assert(buffer->vk.window->vk.isRecording == false);

	Window window = buffer->vk.window;
	Buffer* buffers = window->vk.buffers;
	size_t bufferCount = window->vk.bufferCount;

	for (size_t i = 0; i < bufferCount; i++)
	{
		if (buffer != buffers[i])
			continue;

		for (size_t j = i + 1; j < bufferCount; j++)
			buffers[j - 1] = buffers[j];

		uint8_t api = window->vk.api;

		if (api == VULKAN_GRAPHICS_API)
		{
			destroyVkBuffer(buffer);
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

		window->vk.bufferCount--;
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
const void* getBufferHandle(Buffer buffer)
{
	assert(buffer != NULL);

	uint8_t api = buffer->vk.window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		return &buffer->vk.handle;
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return &buffer->gl.handle;
	}
	else
	{
		abort();
	}
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

	uint8_t api = buffer->vk.window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		setVkBufferData(
			buffer,
			data,
			size,
			offset);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		setGlBufferData(
			buffer,
			data,
			size,
			offset);
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
	assert(window->vk.isRecording == false);

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

	uint8_t api = window->vk.api;

	Mesh mesh;

	if (api == VULKAN_GRAPHICS_API)
	{
		mesh = createVkMesh(
			window,
			drawIndex,
			indexCount,
			indexOffset,
			vertexBuffer,
			indexBuffer);
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

	size_t count = window->vk.meshCount;

	if (count == window->vk.meshCapacity)
	{
		size_t capacity = window->vk.meshCapacity * 2;

		Mesh* meshes = realloc(
			window->vk.meshes,
			sizeof(Mesh) * capacity);

		if (meshes == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
				destroyVkMesh(mesh);
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

		window->vk.meshes = meshes;
		window->vk.meshCapacity = capacity;
	}

	window->vk.meshes[count] = mesh;
	window->vk.meshCount = count + 1;
	return mesh;
}
void destroyMesh(Mesh mesh)
{
	if (mesh == NULL)
		return;

	assert(mesh->vk.window->vk.isRecording == false);

	Window window = mesh->vk.window;
	Mesh* meshes = window->vk.meshes;
	size_t meshCount = window->vk.meshCount;

	for (size_t i = 0; i < meshCount; i++)
	{
		if (mesh != meshes[i])
			continue;

		for (size_t j = i + 1; j < meshCount; j++)
			meshes[j - 1] = meshes[j];

		uint8_t api = window->vk.api;

		if (api == VULKAN_GRAPHICS_API)
		{
			destroyVkMesh(mesh);
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

		window->vk.meshCount--;
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
	assert(mesh->vk.window->vk.isRecording == false);

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
	assert(mesh->vk.window->vk.isRecording == false);

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
	assert(mesh->vk.window->vk.isRecording == false);

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
	assert(mesh->vk.window->vk.isRecording == false);

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
	assert(mesh->vk.window == pipeline->window);
	assert(mesh->vk.window->vk.isRecording == true);

	if (mesh->vk.vertexBuffer == NULL ||
		mesh->vk.indexBuffer == NULL ||
		mesh->vk.indexCount == 0)
	{
		return 0;
	}

	uint8_t api = pipeline->window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		drawVkMesh(
			mesh,
			pipeline);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		drawGlMesh(
			mesh,
			pipeline,
			pipeline->onUniformsSet,
			pipeline->drawMode);
	}
	else
	{
		abort();
	}

	return mesh->vk.indexCount;
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
	assert(window->vk.isRecording == false);

	uint32_t maxImageSize = window->vk.maxImageSize;

	if (size.x > maxImageSize ||
		size.y > maxImageSize ||
		size.z > maxImageSize)
	{
		return NULL;
	}

	uint8_t api = window->vk.api;

	Image image;

	if (api == VULKAN_GRAPHICS_API)
	{
		image = createVkImage(
			window,
			type,
			format,
			size,
			data,
			levelCount);
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

	size_t count = window->vk.imageCount;

	if (count == window->vk.imageCapacity)
	{
		size_t capacity = window->vk.imageCapacity * 2;

		Image* images = realloc(
			window->vk.images,
			sizeof(Image) * capacity);

		if (images == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
				destroyVkImage(image);
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

		window->vk.images = images;
		window->vk.imageCapacity = capacity;
	}

	window->vk.images[count] = image;
	window->vk.imageCount = count + 1;
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
	assert(window->vk.isRecording == false);

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

	assert(image->vk.window->vk.isRecording == false);

	Window window = image->vk.window;
	Image* images = window->vk.images;
	size_t imageCount = window->vk.imageCount;

	for (size_t i = 0; i < imageCount; i++)
	{
		if (image != images[i])
			continue;

		for (size_t j = i + 1; j < imageCount; j++)
			images[j - 1] = images[j];

		uint8_t api = window->vk.api;

		if (api == VULKAN_GRAPHICS_API)
		{
			destroyVkImage(image);
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

		window->vk.imageCount--;
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
	assert(image->vk.window->vk.isRecording == false);

	// TODO: check for static image in Vulkan API

	uint8_t api = image->vk.window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		setVkImageData(
			image,
			data,
			size,
			offset);
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
const void* getImageHandle(Image image)
{
	assert(image != NULL);

	uint8_t api = image->vk.window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		return &image->vk.handle;
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return &image->gl.handle;
	}
	else
	{
		abort();
	}
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
	uint8_t imageCompare,
	bool useCompare,
	float minMipmapLod,
	float maxMipmapLod)
{
	assert(window != NULL);
	assert(minImageFilter < IMAGE_FILTER_COUNT);
	assert(magImageFilter < IMAGE_FILTER_COUNT);
	assert(minMipmapFilter < IMAGE_FILTER_COUNT);
	assert(imageWrapX < IMAGE_WRAP_COUNT);
	assert(imageWrapY < IMAGE_WRAP_COUNT);
	assert(imageWrapZ < IMAGE_WRAP_COUNT);
	assert(imageCompare < IMAGE_COMPARE_COUNT);
	assert(window->vk.isRecording == false);

	uint8_t api = window->vk.api;

	Sampler sampler;

	if (api == VULKAN_GRAPHICS_API)
	{
		sampler = createVkSampler(
			window,
			minImageFilter,
			magImageFilter,
			minMipmapFilter,
			useMipmapping,
			imageWrapX,
			imageWrapY,
			imageWrapZ,
			imageCompare,
			useCompare,
			minMipmapLod,
			maxMipmapLod);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		sampler = createGlSampler(
			window,
			minImageFilter,
			magImageFilter,
			minMipmapFilter,
			useMipmapping,
			imageWrapX,
			imageWrapY,
			imageWrapZ,
			imageCompare,
			useCompare,
			minMipmapLod,
			maxMipmapLod);
	}
	else
	{
		abort();
	}

	if (sampler == NULL)
		return NULL;

	size_t count = window->vk.samplerCount;

	if (count == window->vk.samplerCapacity)
	{
		size_t capacity = window->vk.samplerCapacity * 2;

		Sampler* samplers = realloc(
			window->vk.samplers,
			sizeof(Sampler) * capacity);

		if (samplers == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
				destroyVkSampler(sampler);
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

		window->vk.samplers = samplers;
		window->vk.samplerCapacity = capacity;
	}

	window->vk.samplers[count] = sampler;
	window->vk.samplerCount = count + 1;
	return sampler;
}
void destroySampler(Sampler sampler)
{
	if (sampler == NULL)
		return;

	assert(sampler->vk.window->vk.isRecording == false);

	Window window = sampler->vk.window;
	Sampler* samplers = window->vk.samplers;
	size_t samplerCount = window->vk.samplerCount;

	for (size_t i = 0; i < samplerCount; i++)
	{
		if (sampler != samplers[i])
			continue;

		for (size_t j = i + 1; j < samplerCount; j++)
			samplers[j - 1] = samplers[j];

		uint8_t api = window->vk.api;

		if (api == VULKAN_GRAPHICS_API)
		{
			destroyVkSampler(sampler);
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

		window->vk.samplerCount--;
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
uint8_t getSamplerImageCompare(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.imageCompare;
}
bool isSamplerUseCompare(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.useCompare;
}
float getSamplerMinMipmapLod(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.minMipmapLod;
}
float getSamplerMaxMipmapLod(Sampler sampler)
{
	assert(sampler != NULL);
	return sampler->vk.maxMipmapLod;
}
const void* getSamplerHandle(Sampler sampler)
{
	assert(sampler != NULL);

	uint8_t api = sampler->vk.window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		return &sampler->vk.handle;
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return &sampler->gl.handle;
	}
	else
	{
		abort();
	}
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
	assert(window->vk.isRecording == false);

	uint8_t api = window->vk.api;

	Framebuffer framebuffer;

	if (api == VULKAN_GRAPHICS_API)
	{
		framebuffer = createVkFramebuffer(
			window,
			colorAttachments,
			colorAttachmentCount,
			depthStencilAttachment);
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

	size_t count = window->vk.framebufferCount;

	if (count == window->vk.framebufferCapacity)
	{
		size_t capacity = window->vk.framebufferCapacity * 2;

		Framebuffer* framebuffers = realloc(
			window->vk.framebuffers,
			sizeof(Framebuffer) * capacity);

		if (framebuffers == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
				destroyVkFramebuffer(framebuffer);
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

		window->vk.framebuffers = framebuffers;
		window->vk.framebufferCapacity = capacity;
	}

	window->vk.framebuffers[count] = framebuffer;
	window->vk.framebufferCount = count + 1;
	return framebuffer;
}
void destroyFramebuffer(Framebuffer framebuffer)
{
	if (framebuffer == NULL)
		return;

	assert(framebuffer->vk.window->vk.isRecording == false);

	Window window = framebuffer->vk.window;
	Framebuffer* framebuffers = window->vk.framebuffers;
	size_t framebufferCount = window->vk.framebufferCount;

	for (size_t i = 0; i < framebufferCount; i++)
	{
		if (framebuffer != framebuffers[i])
			continue;

		for (size_t j = i + 1; j < framebufferCount; j++)
			framebuffers[j - 1] = framebuffers[j];

		uint8_t api = window->vk.api;

		if (api == VULKAN_GRAPHICS_API)
		{
			destroyVkFramebuffer(framebuffer);
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

		window->vk.framebufferCount--;
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
	assert(framebuffer->vk.window->vk.isRecording == true);
	assert(framebuffer->vk.window->vk.isRendering == false);

	Window window = framebuffer->vk.window;
	uint8_t api = window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		beginVkFramebufferRender(framebuffer);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		beginGlFramebufferRender(framebuffer);
	}
	else
	{
		abort();
	}

	window->vk.isRendering = true;
}

void endFramebufferRender(Window window)
{
	assert(window != NULL);
	assert(window->vk.isRecording == true);
	assert(window->vk.isRendering == true);

	uint8_t api = window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		endVkFramebufferRender(window);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		endGlFramebufferRender(window);
	}
	else
	{
		abort();
	}

	window->vk.isRendering = false;
}

void clearFramebuffer(
	Window window,
	bool clearColorBuffer,
	bool clearDepthBuffer,
	bool clearStencilBuffer,
	Vec4F clearColor)
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
		clearColorBuffer == true ||
		clearDepthBuffer == true ||
		clearStencilBuffer == true);
	assert(window->vk.isRecording == true);

	uint8_t api = window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		clearVkFramebuffer(
			clearColorBuffer,
			clearDepthBuffer,
			clearStencilBuffer,
			clearColor);
	}
	else if (api == OPENGL_GRAPHICS_API ||
			 api == OPENGL_ES_GRAPHICS_API)
	{
		clearGlFramebuffer(
			clearColorBuffer,
			clearDepthBuffer,
			clearStencilBuffer,
			clearColor);
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
	assert(window->vk.isRecording == false);

	uint8_t api = window->vk.api;

	Shader shader;

	if (api == VULKAN_GRAPHICS_API)
	{
		shader = createVkShader(
			window,
			type,
			code,
			size);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		shader = createGlShader(
			window,
			type,
			code,
			window->vk.api);
	}
	else
	{
		abort();
	}

	if (shader == NULL)
		return NULL;

	size_t count = window->vk.shaderCount;

	if (count == window->vk.shaderCapacity)
	{
		size_t capacity = window->vk.shaderCapacity * 2;

		Shader* shaders = realloc(
			window->vk.shaders,
			sizeof(Shader) * capacity);

		if (shaders == NULL)
		{
			if (api == VULKAN_GRAPHICS_API)
			{
				destroyVkShader(shader);
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

		window->vk.shaders = shaders;
		window->vk.shaderCapacity = capacity;
	}

	window->vk.shaders[count] = shader;
	window->vk.shaderCount = count + 1;
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

	char* code = malloc(fileSize + 1);

	size_t readSize = fread(
		code,
		sizeof(char),
		fileSize,
		file);
	code[fileSize] = '\0';

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

	assert(shader->vk.window->vk.isRecording == false);

	Window window = shader->vk.window;
	Shader* shaders = window->vk.shaders;
	size_t shaderCount = window->vk.shaderCount;

	for (size_t i = 0; i < shaderCount; i++)
	{
		if (shader != shaders[i])
			continue;

		for (size_t j = i + 1; j < shaderCount; j++)
			shaders[j - 1] = shaders[j];

		uint8_t api = window->vk.api;

		if (api == VULKAN_GRAPHICS_API)
		{
			destroyVkShader(shader);
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

		window->vk.shaderCount--;
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
const void* getShaderHandle(Shader shader)
{
	assert(shader != NULL);

	uint8_t api = shader->vk.window->vk.api;

	if (api == VULKAN_GRAPHICS_API)
	{
		return &shader->vk.handle;
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return &shader->gl.handle;
	}
	else
	{
		abort();
	}
}

Pipeline createPipeline(
	Window window,
	const char* name,
	uint8_t drawMode,
	OnPipelineHandleDestroy onHandleDestroy,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	void* handle)
{
	assert(window != NULL);
	assert(name != NULL);
	assert(drawMode < DRAW_MODE_COUNT);
	assert(onHandleDestroy != NULL);
	assert(onHandleBind != NULL);
	assert(onUniformsSet != NULL);
	assert(handle != NULL);
	assert(window->vk.isRecording == false);

	Pipeline pipeline = malloc(
		sizeof(struct Pipeline));

	if (pipeline == NULL)
		return NULL;

	pipeline->window = window;
	pipeline->name = name;
	pipeline->drawMode = drawMode;
	pipeline->onHandleDestroy = onHandleDestroy;
	pipeline->onHandleBind = onHandleBind;
	pipeline->onUniformsSet = onUniformsSet;
	pipeline->handle = handle;

	size_t count = window->vk.pipelineCount;

	if (count == window->vk.pipelineCapacity)
	{
		size_t capacity = window->vk.pipelineCapacity * 2;

		Pipeline* pipelines = realloc(
			window->vk.pipelines,
			sizeof(Pipeline) * capacity);

		if (pipelines == NULL)
		{
			onHandleDestroy(
				window,
				handle);

			free(pipeline);
			return NULL;
		}

		window->vk.pipelines = pipelines;
		window->vk.pipelineCapacity = capacity;
	}

	window->vk.pipelines[count] = pipeline;
	window->vk.pipelineCount = count + 1;
	return pipeline;
}
void destroyPipeline(Pipeline pipeline)
{
	if (pipeline == NULL)
		return;

	assert(pipeline->window->vk.isRecording == false);

	Window window = pipeline->window;
	Pipeline* pipelines = window->vk.pipelines;
	size_t pipelineCount = window->vk.pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		if (pipeline != pipelines[i])
			continue;

		for (size_t j = i + 1; j < pipelineCount; j++)
			pipelines[j - 1] = pipelines[j];

		pipeline->onHandleDestroy(
			window,
			pipeline->handle);
		free(pipeline);
		window->vk.pipelineCount--;
		return;
	}

	abort();
}

Window getPipelineWindow(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->window;
}
const char* getPipelineName(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->name;
}
OnPipelineHandleDestroy getPipelineOnHandleDestroy(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->onHandleDestroy;
}
OnPipelineHandleBind getPipelineOnHandleBind(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->onHandleBind;
}
OnPipelineUniformsSet getPipelineOnUniformsSet(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->onUniformsSet;
}
void* getPipelineHandle(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->handle;
}

uint8_t getPipelineDrawMode(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->drawMode;
}
void setPipelineDrawMode(
	Pipeline pipeline,
	uint8_t drawMode)
{
	assert(pipeline != NULL);
	assert(drawMode < DRAW_MODE_COUNT);
	pipeline->drawMode = drawMode;
}

void bindPipeline(Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->vk.isRecording == true);
	pipeline->onHandleBind(pipeline);
}
