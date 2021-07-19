#include "mpgx/window.h"
#include "mpgx/opengl.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <stdio.h>

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define OPENGL_SHADER_HEADER \
"#version 330 core\n"
// TODO: possibly set default precision
// or detect supported precisions
#define OPENGL_ES_SHADER_HEADER \
"#version 300 es\n"

typedef struct VkBuffer_
{
	Window window;
	uint8_t type;
	size_t size;
	bool isConstant;
	// TODO:
} VkBuffer_;
typedef struct GlBuffer
{
	Window window;
	uint8_t type;
	size_t size;
	bool isConstant;
	GLenum glType;
	GLuint handle;
} GlBuffer;
union Buffer
{
	VkBuffer_ vk;
	GlBuffer gl;
};

typedef struct VkMesh
{
	Window window;
	uint8_t drawIndex;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	// TODO:
} VkMesh;
typedef struct GlMesh
{
	Window window;
	uint8_t drawIndex;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	GLuint handle;
} GlMesh;
union Mesh
{
	VkMesh vk;
	GlMesh gl;
};

typedef struct VkImage_
{
	Window window;
	uint8_t type;
	uint8_t format;
	Vec3U size;
	// TODO:
} VkImage_;
typedef struct GlImage
{
	Window window;
	uint8_t type;
	uint8_t format;
	Vec3U size;
	GLenum glType;
	GLenum dataType;
	GLenum dataFormat;
	GLuint handle;
} GlImage;
union Image
{
	VkImage_ vk;
	GlImage gl;
};

typedef struct VkSampler_
{
	Window window;
	uint8_t minImageFilter;
	uint8_t magImageFilter;
	uint8_t minMipmapFilter;
	bool useMipmapping;
	uint8_t imageWrapX;
	uint8_t imageWrapY;
	uint8_t imageWrapZ;
	uint8_t imageCompare;
	bool useCompare;
	float minMipmapLod;
	float maxMipmapLod;
} VkSampler_;
typedef struct GlSampler
{
	Window window;
	uint8_t minImageFilter;
	uint8_t magImageFilter;
	uint8_t minMipmapFilter;
	bool useMipmapping;
	uint8_t imageWrapX;
	uint8_t imageWrapY;
	uint8_t imageWrapZ;
	uint8_t imageCompare;
	bool useCompare;
	float minMipmapLod;
	float maxMipmapLod;
	GLuint handle;
} GlSampler;
union Sampler
{
	VkSampler_ vk;
	GlSampler gl;
};

union Framebuffer
{
	// TODO:
	void* handle;
};

typedef struct VkShader
{
	Window window;
	uint8_t type;
	// TODO:
} VkShader;
typedef struct GlShader
{
	Window window;
	uint8_t type;
	GLuint handle;
} GlShader;
union Shader
{
	GlShader gl;
	VkShader vk;
};

struct Pipeline
{
	Window window;
	const char* name;
	uint8_t drawMode;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;
	void* handle;
};

struct ImageData
{
	uint8_t* pixels;
	Vec2U size;
	uint8_t channelCount;
};

struct Window
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
};

static bool graphicsInitialized = false;
static FT_Library ftLibrary = NULL;
static Window currentWindow = NULL;

inline static void destroyGlBuffer(Buffer buffer)
{
	makeWindowContextCurrent(
		buffer->gl.window);

	glDeleteBuffers(
		GL_ONE,
		&buffer->gl.handle);
	assertOpenGL();

	free(buffer);
}
inline static void destroyGlMesh(Mesh mesh)
{
	makeWindowContextCurrent(
		mesh->gl.window);

	glDeleteVertexArrays(
		GL_ONE,
		&mesh->gl.handle);
	assertOpenGL();

	free(mesh);
}
inline static void destroyGlImage(Image image)
{
	makeWindowContextCurrent(
		image->gl.window);

	glDeleteTextures(
		GL_ONE,
		&image->gl.handle);
	assertOpenGL();

	free(image);
}
inline static void destroyGlSampler(Sampler sampler)
{
	makeWindowContextCurrent(
		sampler->gl.window);

	glDeleteSamplers(
		GL_ONE,
		&sampler->gl.handle);
	assertOpenGL();

	free(sampler);
}
inline static void destroyGlShader(Shader shader)
{
	makeWindowContextCurrent(
		shader->gl.window);

	glDeleteShader(shader->gl.handle);
	assertOpenGL();

	free(shader);
}

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

	glfwSetErrorCallback(
		glfwErrorCallback);

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
	assert(size.x > 0);
	assert(size.y > 0);
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
		sizeof(struct Window));

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

		// TODO: add Vulkan support
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

	if (glfwRawMouseMotionSupported() == GLFW_TRUE)
	{
		glfwSetInputMode(
			handle,
			GLFW_RAW_MOUSE_MOTION,
			GLFW_TRUE);
	}

	if (api == VULKAN_GRAPHICS_API)
	{
		// TODO: implement Vulkan functions
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

		GLint maxImageSize;

		glGetIntegerv(
			GL_MAX_TEXTURE_SIZE,
			&maxImageSize);
		assertOpenGL();

		window->maxImageSize = maxImageSize;
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

	window->api = api;
	window->onUpdate = onUpdate;
	window->updateArgument = updateArgument;
	window->handle = handle;
	window->buffers = buffers;
	window->bufferCapacity = bufferCapacity;
	window->bufferCount = 0;
	window->meshes = meshes;
	window->meshCapacity = meshCapacity;
	window->meshCount = 0;
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
	window->updateTime = 0.0;
	window->deltaTime = 0.0;
	window->isRecording = false;

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
	Mesh* meshes = window->meshes;
	size_t meshCount = window->meshCount;
	Buffer* buffers = window->buffers;
	size_t bufferCount = window->bufferCount;

	uint8_t api = window->api;

    if (api == VULKAN_GRAPHICS_API)
	{
    	abort();
	}
    else if (api == OPENGL_GRAPHICS_API ||
    	api == OPENGL_ES_GRAPHICS_API)
	{
		for (size_t i = 0; i < pipelineCount; i++)
		{
			Pipeline pipeline = pipelines[i];

			pipeline->onDestroy(
				window,
				pipeline->handle);
		}

		for (size_t i = 0; i < shaderCount; i++)
			destroyGlShader(shaders[i]);
		/*for (size_t i = 0; i < framebufferCount; i++)
			destroyGlFramebuffer(framebuffers[i]);*/ // TODO:
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

	free(buffers);
	free(meshes);
	free(images);
	free(framebuffers);
	free(shaders);
	free(pipelines);
	glfwDestroyWindow(window->handle);
	free(window);
}

uint8_t getWindowGraphicsAPI(Window window)
{
	assert(window != NULL);
	return window->api;
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
uint32_t getWindowMaxImageSize(Window window)
{
	assert(window != NULL);
	return window->maxImageSize;
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

	int width, height;

	glfwGetFramebufferSize(
		window->handle,
		&width,
		&height);

	return vec2U(width, height);
}
const char* getWindowClipboard(Window window)
{
	assert(window != NULL);
	return glfwGetClipboardString(window->handle);
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

inline static void beginGlWindowRender(Window window)
{
	int width, height;

	glfwGetFramebufferSize(
		window->handle,
		&width,
		&height);

	glViewport(0, 0, width, height);

	// TODO: move to the framebuffer
	glClear(
		GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);
}
void beginWindowRender(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == false);

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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

	window->isRecording = true;
}

inline static void endGlWindowRender(Window window)
{
	glfwSwapBuffers(window->handle);
}
void endWindowRender(Window window)
{
	assert(window != NULL);
	assert(window->isRecording == true);

	uint8_t api = window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
		return;
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

	window->isRecording = false;
}

inline static Buffer createGlBuffer(
	Window window,
	uint8_t type,
	const void* data,
	size_t size,
	bool isConstant)
{
	Buffer buffer = malloc(
		sizeof(union Buffer));

	if (buffer == NULL)
		return NULL;

	GLenum glType;

	if (type == VERTEX_BUFFER_TYPE)
	{
		glType = GL_ARRAY_BUFFER;
	}
	else if (type == INDEX_BUFFER_TYPE)
	{
		glType = GL_ELEMENT_ARRAY_BUFFER;
	}
	else if (type == UNIFORM_BUFFER_TYPE)
	{
		glType = GL_UNIFORM_BUFFER;
	}
	else
	{
		free(buffer);
		return NULL;
	}

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenBuffers(
		GL_ONE,
		&handle);

	GLenum usage = isConstant ?
		GL_STATIC_DRAW :
		GL_DYNAMIC_DRAW;

	glBindBuffer(
		glType,
		handle);
	glBufferData(
		glType,
		(GLsizeiptr)(size),
		data,
		usage);

	assertOpenGL();

	buffer->gl.window = window;
	buffer->gl.type = type;
	buffer->gl.size = size;
	buffer->gl.isConstant = isConstant;
	buffer->gl.glType = glType;
	buffer->gl.handle = handle;
	return buffer;
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
		return NULL;
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
				abort();
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
			abort();
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
const void* getBufferHandle(Buffer buffer)
{
	assert(buffer != NULL);

	uint8_t api = buffer->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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

inline static void setGlBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	makeWindowContextCurrent(
		buffer->gl.window);

	glBindBuffer(
		buffer->gl.glType,
		buffer->gl.handle);
	glBufferSubData(
		buffer->gl.glType,
		(GLintptr)offset,
		(GLsizeiptr)size,
		data);

	assertOpenGL();
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
		abort();
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

inline static Mesh createGlMesh(
	Window window,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	Mesh mesh = malloc(
		sizeof(union Mesh));

	if (mesh == NULL)
		return NULL;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenVertexArrays(
		GL_ONE,
		&handle);
	assertOpenGL();

	mesh->gl.window = window;
	mesh->gl.drawIndex = drawIndex;
	mesh->gl.indexCount = indexCount;
	mesh->gl.indexOffset = indexOffset;
	mesh->gl.vertexBuffer = vertexBuffer;
	mesh->gl.indexBuffer = indexBuffer;
	mesh->gl.handle = handle;
	return mesh;
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
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);
	assert(window == vertexBuffer->vk.window);
	assert(window == indexBuffer->vk.window);
	assert(vertexBuffer->vk.type == VERTEX_BUFFER_TYPE);
	assert(indexBuffer->vk.type == INDEX_BUFFER_TYPE);
	assert(window->isRecording == false);

#ifndef NDEBUG
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
#endif

	uint8_t api = window->api;

	Mesh mesh;

	if (api == VULKAN_GRAPHICS_API)
	{
		return NULL;
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
				abort();
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
void destroyMesh(Mesh mesh)
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

		uint8_t api = window->api;

		if (api == VULKAN_GRAPHICS_API)
		{
			abort();
		}
		else if (api == OPENGL_GRAPHICS_API)
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
	assert(vertexBuffer != NULL);
	assert(mesh->vk.window == vertexBuffer->vk.window);
	assert(vertexBuffer->vk.type == VERTEX_BUFFER_TYPE);
	assert(mesh->vk.window->isRecording == false);
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
	assert(indexCount != 0);
	assert(indexBuffer != NULL);
	assert(mesh->vk.window == indexBuffer->vk.window);
	assert(indexBuffer->vk.type == INDEX_BUFFER_TYPE);
	assert(mesh->vk.window->isRecording == false);

#ifndef NDEBUG
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
#endif

	mesh->vk.drawIndex = drawIndex;
	mesh->vk.indexCount = indexCount;
	mesh->vk.indexOffset = indexOffset;
	mesh->vk.indexBuffer = indexBuffer;
}

inline static void drawGlMesh(
	Mesh mesh,
	Pipeline pipeline)
{
	Buffer vertexBuffer = mesh->gl.vertexBuffer;
	Buffer indexBuffer = mesh->gl.indexBuffer;

	glBindVertexArray(
		mesh->gl.handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		vertexBuffer->gl.handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		indexBuffer->gl.handle);
	assertOpenGL();

	pipeline->onUniformsSet(pipeline);

	GLenum glDrawMode;

	switch (pipeline->drawMode)
	{
	default:
		abort();
	case POINTS_DRAW_MODE:
		glDrawMode = GL_POINTS;
		break;
	case LINE_STRIP_DRAW_MODE:
		glDrawMode = GL_LINE_STRIP;
		break;
	case LINE_LOOP_DRAW_MODE:
		glDrawMode = GL_LINE_LOOP;
		break;
	case LINES_DRAW_MODE:
		glDrawMode = GL_LINES;
		break;
	case TRIANGLE_STRIP_DRAW_MODE:
		glDrawMode = GL_TRIANGLE_STRIP;
		break;
	case TRIANGLE_FAN_DRAW_MODE:
		glDrawMode = GL_TRIANGLE_FAN;
		break;
	case TRIANGLES_DRAW_MODE:
		glDrawMode = GL_TRIANGLES;
		break;
	}

	uint8_t drawIndex = mesh->gl.drawIndex;

	GLenum glDrawIndex;
	size_t glIndexOffset;

	if (drawIndex == UINT16_DRAW_INDEX)
	{
		glDrawIndex = GL_UNSIGNED_SHORT;
		glIndexOffset = mesh->gl.indexOffset * sizeof(uint16_t);
	}
	else if (drawIndex == UINT32_DRAW_INDEX)
	{
		glDrawIndex = GL_UNSIGNED_INT;
		glIndexOffset = mesh->gl.indexOffset * sizeof(uint32_t);
	}
	else
	{
		abort();
	}

	glDrawElements(
		glDrawMode,
		(GLsizei)mesh->gl.indexCount,
		glDrawIndex,
		(const void*)glIndexOffset);

	assertOpenGL();
}

void drawMesh(
	Mesh mesh,
	Pipeline pipeline)
{
	assert(mesh != NULL);
	assert(pipeline != NULL);
	assert(mesh->vk.window == pipeline->window);
	assert(mesh->vk.window->isRecording == true);

	uint8_t api = pipeline->window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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

inline static Image createGlImage(
	Window window,
	uint8_t type,
	uint8_t format,
	Vec3U size,
	const void** data,
	uint8_t levelCount)
{
	Image image = malloc(
		sizeof(union Image));

	if (image == NULL)
		return NULL;

	GLenum glType;
	GLenum dataFormat;
	GLenum dataType;

	if (type == IMAGE_2D_TYPE)
	{
		glType = GL_TEXTURE_2D;
	}
	else if (type == IMAGE_3D_TYPE)
	{
		glType = GL_TEXTURE_3D;
	}
	else
	{
		abort();
	}

	GLint glFormat;

	switch (format)
	{
	default:
		free(image);
		return NULL;
	case R8G8B8A8_UNORM_IMAGE_FORMAT:
		glFormat = GL_RGBA8;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case R8G8B8A8_SRGB_IMAGE_FORMAT:
		glFormat = GL_SRGB8_ALPHA8;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	}

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenTextures(
		GL_ONE,
		&handle);
	glBindTexture(
		glType,
		handle);

	if (type == IMAGE_2D_TYPE)
	{
		if (levelCount == 0)
		{
			glTexImage2D(
				glType,
				0,
				glFormat,
				(GLsizei)size.x,
				(GLsizei)size.y,
				0,
				dataFormat,
				dataType,
				data[0]);
			glGenerateMipmap(glType);
		}
		else
		{
			Vec2U mipSize = vec2U(size.x, size.y);

			for (uint8_t i = 0; i < levelCount; i++)
			{
				glTexImage2D(
					glType,
					(GLint)i,
					glFormat,
					(GLsizei)mipSize.x,
					(GLsizei)mipSize.y,
					0,
					dataFormat,
					dataType,
					data[i]);

				mipSize = vec2U(
					mipSize.x / 2,
					mipSize.y / 2);
			}

			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_BASE_LEVEL,
				0);
			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_MAX_LEVEL,
				levelCount - 1);
		}
	}
	else
	{
		if (levelCount == 0)
		{
			glTexImage3D(
				glType,
				0,
				glFormat,
				(GLsizei)size.x,
				(GLsizei)size.y,
				(GLsizei)size.z,
				0,
				dataFormat,
				dataType,
				data[0]);
			glGenerateMipmap(glType);
		}
		else
		{
			Vec3U mipSize = size;

			for (uint8_t i = 0; i < levelCount; i++)
			{
				glTexImage3D(
					glType,
					(GLint)i,
					glFormat,
					(GLsizei)mipSize.x,
					(GLsizei)mipSize.y,
					(GLsizei)mipSize.z,
					0,
					dataFormat,
					dataType,
					data[i]);

				mipSize = vec3U(
					mipSize.x / 2,
					mipSize.y / 2,
					mipSize.z / 2);
			}

			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_BASE_LEVEL,
				0);
			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_MAX_LEVEL,
				levelCount - 1);
		}
	}

	assertOpenGL();

	image->gl.window = window;
	image->gl.type = type;
	image->gl.format = format;
	image->gl.size = size;
	image->gl.handle = handle;
	image->gl.glType = glType;
	image->gl.dataType = dataType;
	image->gl.dataFormat = dataFormat;
	image->gl.handle = handle;
	return image;
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

	uint32_t maxImageSize = window->maxImageSize;

	if (size.x > maxImageSize ||
		size.y > maxImageSize ||
		size.z > maxImageSize)
	{
		return NULL;
	}

	uint8_t api = window->api;

	Image image;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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
				abort();
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
			abort();
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

inline static void setGlImageData(
	Image image,
	const void* data,
	Vec3U size,
	Vec3U offset)
{
	makeWindowContextCurrent(
		image->gl.window);

	glBindTexture(
		image->gl.glType,
		image->gl.handle);

	uint8_t type = image->gl.type;

	if (type == IMAGE_2D_TYPE)
	{
		glTexSubImage2D(
			image->gl.glType,
			0,
			(GLint)offset.x,
			(GLint)offset.y,
			(GLsizei)size.x,
			(GLsizei)size.y,
			image->gl.dataFormat,
			image->gl.dataType,
			data);
	}
	else if (type == IMAGE_3D_TYPE)
	{
		glTexSubImage3D(
			image->gl.glType,
			0,
			(GLint)offset.x,
			(GLint)offset.y,
			(GLint)offset.z,
			(GLsizei)size.x,
			(GLsizei)size.y,
			(GLsizei)size.z,
			image->gl.dataFormat,
			image->gl.dataType,
			data);
	}
	else
	{
		abort();
	}

	assertOpenGL();
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
		abort();
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

	uint8_t api = image->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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

inline static Sampler createGlSampler(
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
	Sampler sampler = malloc(
		sizeof(union Sampler));

	if (sampler == NULL)
		return NULL;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenSamplers(
		GL_ONE,
		&handle);

	glSamplerParameteri(
		handle,
		GL_TEXTURE_MIN_FILTER,
		(GLint)getGlImageFilter(
			minImageFilter,
			minMipmapFilter,
			useMipmapping));
	glSamplerParameteri(
		handle,
		GL_TEXTURE_MAG_FILTER,
		(GLint)getGlImageFilter(
			magImageFilter,
			magImageFilter,
			false));

	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_S,
		(GLint)getGlImageWrap(imageWrapX));
	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_T,
		(GLint)getGlImageWrap(imageWrapY));
	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_R,
		(GLint)getGlImageWrap(imageWrapZ));

	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_MODE,
		useCompare ?
			GL_COMPARE_REF_TO_TEXTURE :
			GL_NONE);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_FUNC,
		(GLint)getGlImageCompare(imageCompare));

	glSamplerParameterf(
		handle,
		GL_TEXTURE_MIN_LOD,
		(GLfloat)minMipmapLod);
	glSamplerParameterf(
		handle,
		GL_TEXTURE_MAX_LOD,
		(GLfloat)maxMipmapLod);

	assertOpenGL();

	sampler->gl.window = window;
	sampler->gl.minImageFilter = minImageFilter;
	sampler->gl.magImageFilter = magImageFilter;
	sampler->gl.minMipmapFilter = minMipmapFilter;
	sampler->gl.useMipmapping = useMipmapping;
	sampler->gl.imageWrapX = imageWrapX;
	sampler->gl.imageWrapY = imageWrapY;
	sampler->gl.imageWrapZ = imageWrapZ;
	sampler->gl.imageCompare = imageCompare;
	sampler->gl.useCompare = useCompare;
	sampler->gl.minMipmapLod = minMipmapLod;
	sampler->gl.maxMipmapLod = maxMipmapLod;
	sampler->gl.handle = handle;
	return sampler;
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
	assert(window->isRecording == false);

	uint8_t api = window->api;

	Sampler sampler;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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
				abort();
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
			abort();
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

	uint8_t api = sampler->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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

inline static Shader createGlShader(
	Window window,
	uint8_t type,
	const void* code,
	size_t size)
{
	Shader shader = malloc(
		sizeof(union Shader));

	if (shader == NULL)
		return NULL;

	GLenum glType;

	if (type == VERTEX_SHADER_TYPE)
		glType = GL_VERTEX_SHADER;
	else if (type == FRAGMENT_SHADER_TYPE)
		glType = GL_FRAGMENT_SHADER;
	else if (type == COMPUTE_SHADER_TYPE)
		glType = GL_COMPUTE_SHADER;
	else
		abort();

	uint8_t api = window->api;

	const char* sources[2];

	if (api == OPENGL_GRAPHICS_API)
		sources[0] = OPENGL_SHADER_HEADER;
	else if (api == OPENGL_ES_GRAPHICS_API)
		sources[0] = OPENGL_ES_SHADER_HEADER;
	else
		abort();

	sources[1] = (const char*)code;

	makeWindowContextCurrent(window);

	GLuint handle = glCreateShader(glType);

	glShaderSource(
		handle,
		2,
		sources,
		NULL);

	glCompileShader(handle);

	GLint result;

	glGetShaderiv(
		handle,
		GL_COMPILE_STATUS,
		&result);

	if (result == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetShaderiv(
			handle,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				glDeleteShader(handle);
				free(shader);
				return NULL;
			}

			glGetShaderInfoLog(
				handle,
				length,
				&length,
				(GLchar*)infoLog);

			const char* typeString;

			if (type == VERTEX_SHADER_TYPE)
				typeString = "vertex";
			else if (type == FRAGMENT_SHADER_TYPE)
				typeString = "fragment";
			else if (type == COMPUTE_SHADER_TYPE)
				typeString = "compute";
			else
				abort();

			fprintf(stderr,
				"OpenGL %s shader compile error: %s\n",
				typeString,
				infoLog);
			free(infoLog);
		}
#endif

		assertOpenGL();

		glDeleteShader(handle);
		free(shader);
		return NULL;
	}

	assertOpenGL();

	shader->gl.window = window;
	shader->gl.type = type;
	shader->gl.handle = handle;
	return shader;
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
		abort();
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		shader = createGlShader(
			window,
			type,
			code,
			size);
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
				abort();
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
			abort();
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
const void* getShaderHandle(Shader shader)
{
	assert(shader != NULL);

	uint8_t api = shader->vk.window->api;

	if (api == VULKAN_GRAPHICS_API)
	{
		abort();
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
	OnPipelineDestroy onDestroy,
	OnPipelineBind onBind,
	OnPipelineUniformsSet onUniformsSet,
	void* handle)
{
	assert(window != NULL);
	assert(name != NULL);
	assert(drawMode < DRAW_MODE_COUNT);
	assert(onDestroy != NULL);
	assert(onBind != NULL);
	assert(onUniformsSet != NULL);
	assert(handle != NULL);
	assert(window->isRecording == false);

	Pipeline pipeline = malloc(
		sizeof(struct Pipeline));

	if (pipeline == NULL)
		return NULL;

	pipeline->window = window;
	pipeline->name = name;
	pipeline->drawMode = drawMode;
	pipeline->onDestroy = onDestroy;
	pipeline->onBind = onBind;
	pipeline->onUniformsSet = onUniformsSet;
	pipeline->handle = handle;

	size_t count = window->pipelineCount;

	if (count == window->pipelineCapacity)
	{
		size_t capacity = window->pipelineCapacity * 2;

		Pipeline* pipelines = realloc(
			window->pipelines,
			sizeof(Pipeline) * capacity);

		if (pipelines == NULL)
		{
			onDestroy(
				window,
				handle);

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
void destroyPipeline(Pipeline pipeline)
{
	if (pipeline == NULL)
		return;

	assert(pipeline->window->isRecording == false);

	Window window = pipeline->window;
	Pipeline* pipelines = window->pipelines;
	size_t pipelineCount = window->pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		if (pipeline != pipelines[i])
			continue;

		for (size_t j = i + 1; j < pipelineCount; j++)
			pipelines[j - 1] = pipelines[j];

		pipeline->onDestroy(
			window,
			pipeline->handle);
		free(pipeline);
		window->pipelineCount--;
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
OnPipelineDestroy getPipelineOnDestroy(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->onDestroy;
}
OnPipelineBind getPipelineOnBind(Pipeline pipeline)
{
	assert(pipeline != NULL);
	return pipeline->onBind;
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
	assert(pipeline->window->isRecording == true);
	pipeline->onBind(pipeline);
}
