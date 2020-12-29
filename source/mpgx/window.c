#include "mpgx/window.h"
#include "mpgx/pipeline.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include <stdio.h>

#define OPENGL_SHADER_HEADER \
"#version 330 core\n\n#define highp \n#define mediump \n#define lowp \n"
#define OPENGL_ES_SHADER_HEADER \
"#version 300 es\n"

#if __linux__ || __APPLE__
#define fseek(stream, offset, origin) \
fseeko(stream, offset, origin)
#endif

typedef void(*BeginCommandRecord)(
	struct Window*);
typedef void(*EndCommandRecord)(
	struct Window*);

typedef void*(*CreateBuffer)(
	struct Window*,
	uint8_t,
	const void*,
	size_t,
	bool);
typedef void(*DestroyBuffer)(
	struct Window*,
	void*);
typedef void(*SetBufferData)(
	struct Buffer*,
	const void*,
	size_t,
	size_t);

typedef void*(*CreateMesh)(
	struct Window*);
typedef void(*DestroyMesh)(
	struct Window*,
	void*);
typedef void(*DrawMeshCommand)(
	struct Mesh*,
	struct Pipeline*);

typedef void*(*CreateImage)(
	struct Window*,
	uint8_t,
	uint8_t,
	size_t,
	size_t,
	size_t,
	const void*,
	bool);
typedef void(*DestroyImage)(
	struct Window*,
	void*);
typedef void(*SetImageData)(
	struct Image*,
	const void*,
	size_t,
	size_t,
	size_t,
	size_t,
	size_t,
	size_t,
	size_t);
typedef void(*GenerateMipmap)(
	struct Image*);
typedef const void*(*GetImageHandle)(
	const struct Image*);

typedef void*(*CreateFramebuffer)(
	struct Window*);
typedef void(*DestroyFramebuffer)(
	struct Window*,
	void*);
typedef void*(*CreateShader)(
	struct Window*,
	uint8_t,
	const void*,
	size_t);
typedef void(*DestroyShader)(
	struct Window*,
	void*);
typedef const void*(*GetShaderHandle)(
	const struct Shader*);

struct Window
{
	uint8_t api;
	size_t maxImageSize;
	UpdateWindow updateFunction;
	void* updateArgument;
	BeginCommandRecord beginRecordFunction;
	BeginCommandRecord endRecordFunction;
	CreateBuffer createBufferFunction;
	DestroyBuffer destroyBufferFunction;
	SetBufferData setBufferDataFunction;
	CreateMesh createMeshFunction;
	DestroyMesh destroyMeshFunction;
	DrawMeshCommand drawMeshFunction;
	CreateImage createImageFunction;
	DestroyImage destroyImageFunction;
	SetImageData setImageDataFunction;
	GenerateMipmap generateMipmapFunction;
	GetImageHandle getImageHandleFunction;
	CreateFramebuffer createFramebufferFunction;
	DestroyFramebuffer destroyFramebufferFunction;
	CreateShader createShaderFunction;
	DestroyShader destroyShaderFunction;
	GetShaderHandle getShaderHandleFunction;
	struct Buffer** buffers;
	size_t bufferCapacity;
	size_t bufferCount;
	struct Mesh** meshes;
	size_t meshCapacity;
	size_t meshCount;
	struct Image** images;
	size_t imageCapacity;
	size_t imageCount;
	struct Framebuffer** framebuffers;
	size_t framebufferCapacity;
	size_t framebufferCount;
	struct Shader** shaders;
	size_t shaderCapacity;
	size_t shaderCount;
	struct Pipeline** pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	struct GLFWwindow* handle;
	double updateTime;
	double deltaTime;
	bool recording;
};

struct GlBuffer
{
	GLenum type;
	GLuint handle;
};
struct Buffer
{
	struct Window* window;
	uint8_t type;
	size_t size;
	bool constant;
	void* handle;
};

struct GlMesh
{
	GLuint handle;
};
struct Mesh
{
	struct Window* window;
	uint8_t drawIndex;
	size_t indexCount;
	struct Buffer* vertexBuffer;
	struct Buffer* indexBuffer;
	void* handle;
};

struct GlImage
{
	GLenum type;
	GLenum dataType;
	GLenum dataFormat;
	GLuint handle;
};
struct Image
{
	struct Window* window;
	uint8_t type;
	uint8_t format;
	size_t width;
	size_t height;
	size_t depth;
	bool mipmap;
	void* handle;
};

struct Framebuffer
{
	// TODO:
	void* handle;
};

struct GlShader
{
	GLuint handle;
};
struct Shader
{
	struct Window* window;
	uint8_t type;
	void* handle;
};

struct Pipeline
{
	struct Window* window;
	uint8_t drawMode;
	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;
	void* handle;
};

static bool graphicsInitialized = false;
static FT_Library ftLibrary = NULL;

void beginGlCommandRecord(
	struct Window* window)
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
void endGlCommandRecord(
	struct Window* window)
{
	glfwSwapBuffers(window->handle);
}

void* createGlBuffer(
	struct Window* window,
	uint8_t _type,
	const void* data,
	size_t size,
	bool constant)
{
	struct GlBuffer* buffer = malloc(
		sizeof(struct GlBuffer));

	if (buffer == NULL)
		return NULL;

	GLenum type;

	if (_type == VERTEX_BUFFER_TYPE)
	{
		type = GL_ARRAY_BUFFER;
	}
	else if (_type == INDEX_BUFFER_TYPE)
	{
		type = GL_ELEMENT_ARRAY_BUFFER;
	}
	else if (_type == UNIFORM_BUFFER_TYPE)
	{
		type = GL_UNIFORM_BUFFER;
	}
	else
	{
		free(buffer);
		return NULL;
	}

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = GL_ZERO;

	glGenBuffers(
		GL_ONE,
		&handle);

	GLenum usage = constant ?
		GL_STATIC_DRAW :
		GL_DYNAMIC_DRAW;

	glBindBuffer(
		type,
		handle);
	glBufferData(
		type,
		(GLsizeiptr)(size),
		data,
		usage);

	assertOpenGL();

	buffer->type = type;
	buffer->handle = handle;
	return buffer;
}
void destroyGlBuffer(
	struct Window* window,
	void* buffer)
{
	struct GlBuffer* glBuffer =
		(struct GlBuffer*)buffer;

	glfwMakeContextCurrent(
		window->handle);

	glDeleteBuffers(
		GL_ONE,
		&glBuffer->handle);

	assertOpenGL();

	free(glBuffer);
}
void setGlBufferData(
	struct Buffer* buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	struct GlBuffer* glBuffer =
		(struct GlBuffer*)buffer->handle;

	glfwMakeContextCurrent(
		buffer->window->handle);

	glBindBuffer(
		glBuffer->type,
		glBuffer->handle);
	glBufferSubData(
		glBuffer->type,
		(GLintptr)offset,
		(GLsizeiptr)size,
		data);

	assertOpenGL();
}

void* createGlMesh(
	struct Window* window)
{
	struct GlMesh* mesh = malloc(
		sizeof(struct GlMesh));

	if (mesh == NULL)
		return NULL;

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = GL_ZERO;

	glGenVertexArrays(
		GL_ONE,
		&handle);

	assertOpenGL();

	mesh->handle = handle;
	return mesh;
}
void destroyGlMesh(
	struct Window* window,
	void* mesh)
{
	struct GlMesh* glMesh =
		(struct GlMesh*)mesh;

	glfwMakeContextCurrent(
		window->handle);

	glDeleteVertexArrays(
		GL_ONE,
		&glMesh->handle);

	assertOpenGL();

	free(glMesh);
}
void drawGlMeshCommand(
	struct Mesh* mesh,
	struct Pipeline* pipeline)
{
	struct GlMesh* glMesh =
		(struct GlMesh*)mesh->handle;
	struct GlBuffer* glVertexBuffer =
		(struct GlBuffer*)mesh->vertexBuffer->handle;
	struct GlBuffer* glIndexBuffer =
		(struct GlBuffer*)mesh->indexBuffer->handle;

	glBindVertexArray(
		glMesh->handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		glVertexBuffer->handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		glIndexBuffer->handle);

	pipeline->setUniformsFunction(
		pipeline);

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

	GLenum glDrawIndex =
		mesh->drawIndex == UINT32_DRAW_INDEX ?
			GL_UNSIGNED_INT :
			GL_UNSIGNED_SHORT;

	glDrawElements(
		glDrawMode,
		(GLsizei)mesh->indexCount,
		glDrawIndex,
		NULL);

	assertOpenGL();
}

void* createGlImage(
	struct Window* window,
	uint8_t _type,
	uint8_t _format,
	size_t width,
	size_t height,
	size_t depth,
	const void* pixels,
	bool mipmap)
{
	struct GlImage* image = malloc(
		sizeof(struct GlImage));

	if (image == NULL)
		return NULL;

	GLenum type;
	GLenum dataFormat;
	GLenum dataType;

	if (_type == IMAGE_2D_TYPE)
	{
		type = GL_TEXTURE_2D;
	}
	else if (_type == IMAGE_3D_TYPE)
	{
		type = GL_TEXTURE_3D;
	}
	else
	{
		free(image);
		return NULL;
	}

	GLenum format;

	switch (_format)
	{
	default:
		free(image);
		return NULL;
	case R8G8B8A8_UNORM_IMAGE_FORMAT:
		format = GL_RGBA8;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case R8G8B8A8_SRGB_IMAGE_FORMAT:
		format = GL_SRGB8_ALPHA8;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	}

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = GL_ZERO;

	glGenTextures(
		GL_ONE,
		&handle);

	glBindTexture(
		type,
		handle);

	if (_type == IMAGE_2D_TYPE)
	{
		glTexImage2D(
			type,
			0,
			format,
			(GLsizei)width,
			(GLsizei)height,
			0,
			dataFormat,
			dataType,
			pixels);
	}
	else
	{
		glTexImage3D(
			type,
			0,
			format,
			(GLsizei)width,
			(GLsizei)height,
			(GLsizei)depth,
			0,
			dataFormat,
			dataType,
			pixels);
	}

	if (mipmap == true)
		glGenerateMipmap(type);

	assertOpenGL();

	image->type = type;
	image->dataType = dataType;
	image->dataFormat = dataFormat;
	image->handle = handle;
	return image;
}
void destroyGlImage(
	struct Window* window,
	void* image)
{
	struct GlImage* glImage =
		(struct GlImage*)image;

	glfwMakeContextCurrent(
		window->handle);

	glDeleteTextures(
		GL_ONE,
		&glImage->handle);

	assertOpenGL();

	free(glImage);
}
void setGlImageData(
	struct Image* image,
	const void* data,
	size_t width,
	size_t height,
	size_t depth,
	size_t widthOffset,
	size_t heightOffset,
	size_t depthOffset,
	size_t mipmapLevel)
{
	struct GlImage* glImage =
		(struct GlImage*)image->handle;

	glfwMakeContextCurrent(
		image->window->handle);

	glBindTexture(
		glImage->type,
		glImage->handle);

	if (image->type == IMAGE_2D_TYPE)
	{
		glTexSubImage2D(
			glImage->type,
			(GLint)mipmapLevel,
			(GLint)widthOffset,
			(GLint)heightOffset,
			(GLsizei)width,
			(GLsizei)height,
			glImage->dataFormat,
			glImage->dataType,
			data);
	}
	else if (image->type == IMAGE_3D_TYPE)
	{
		glTexSubImage3D(
			glImage->type,
			(GLint)mipmapLevel,
			(GLint)widthOffset,
			(GLint)heightOffset,
			(GLint)depthOffset,
			(GLsizei)width,
			(GLsizei)height,
			(GLsizei)depth,
			glImage->dataFormat,
			glImage->dataType,
			data);
	}
	else
	{
		abort();
	}

	assertOpenGL();
}
void generateGlMipmap(
	struct Image* image)
{
	struct GlImage* glImage =
		(struct GlImage*)image->handle;

	glfwMakeContextCurrent(
		image->window->handle);

	glBindTexture(
		glImage->type,
		glImage->handle);
	glGenerateMipmap(
		glImage->type);

	assertOpenGL();
}
const void* getGlImageHandle(
	const struct Image* image)
{
	struct GlImage* glImage =
		(struct GlImage*)image->handle;
	return &glImage->handle;
}

void* createGlShader(
	struct Window* window,
	uint8_t _type,
	const void* code,
	size_t size)
{
	struct GlShader* shader = malloc(
		sizeof(struct GlShader));

	if (shader == NULL)
		return NULL;

	GLenum type;

	if (_type == VERTEX_SHADER_TYPE)
	{
		type = GL_VERTEX_SHADER;
	}
	else if (_type == FRAGMENT_SHADER_TYPE)
	{
		type = GL_FRAGMENT_SHADER;
	}
	else if (_type == COMPUTE_SHADER_TYPE)
	{
		type = GL_COMPUTE_SHADER;
	}
	else
	{
		free(shader);
		return NULL;
	}

	// TODO: could be optimized by selecting
	// createGlShader and createGlesShader
	uint8_t api = getWindowGraphicsAPI(window);

	const char* sources[2];

	if (api == OPENGL_GRAPHICS_API)
		sources[0] = OPENGL_SHADER_HEADER;
	else
		sources[0] = OPENGL_ES_SHADER_HEADER;

	sources[1] = (const char*)code;

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = glCreateShader(type);

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
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		assertOpenGL();

		glDeleteShader(handle);
		free(shader);
		return NULL;
	}

	assertOpenGL();

	shader->handle = handle;
	return shader;
}
void destroyGlShader(
	struct Window* window,
	void* shader)
{
	struct GlShader* glShader =
		(struct GlShader*)shader;

	glfwMakeContextCurrent(
		window->handle);

	glDeleteShader(
		glShader->handle);

	assertOpenGL();

	free(glShader);
}
const void* getGlShaderHandle(
	const struct Shader* shader)
{
	struct GlShader* glShader =
		(struct GlShader*)shader->handle;
	return &glShader->handle;
}

void glfwErrorCallback(
	int error,
	const char* description)
{
	fprintf(
		stderr,
		"GLFW ERROR: %s\n",
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

void* getFtLibrary()
{
	return ftLibrary;
}

struct Window* createWindow(
	uint8_t api,
	size_t width,
	size_t height,
	const char* title,
	UpdateWindow updateFunction,
	void* updateArgument)
{
	assert(width != 0);
	assert(height != 0);
	assert(title != NULL);
	assert(updateFunction != NULL);
	assert(graphicsInitialized == true);

	struct Window* window =
		malloc(sizeof(struct Window));

	if (window == NULL)
		return NULL;

	glfwDefaultWindowHints();

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
		free(window);
		return NULL;
	}

	struct Buffer** buffers = malloc(
		sizeof(struct Buffer*));

	if (buffers == NULL)
	{
		free(window);
		return NULL;
	}

	struct Mesh** meshes = malloc(
		sizeof(struct Mesh*));

	if (meshes == NULL)
	{
		free(buffers);
		free(window);
		return NULL;
	}

	struct Image** images = malloc(
		sizeof(struct Image*));

	if (images == NULL)
	{
		free(meshes);
		free(buffers);
		free(window);
		return NULL;
	}

	struct Framebuffer** framebuffers = malloc(
		sizeof(struct Framebuffer*));

	if (images == NULL)
	{
		free(images);
		free(meshes);
		free(buffers);
		free(window);
		return NULL;
	}

	struct Shader** shaders = malloc(
		sizeof(struct Shader*));

	if (shaders == NULL)
	{
		free(framebuffers);
		free(images);
		free(meshes);
		free(buffers);
		free(window);
		return NULL;
	}

	struct Pipeline** pipelines = malloc(
		sizeof(struct Pipeline*));

	if (pipelines == NULL)
	{
		free(shaders);
		free(framebuffers);
		free(images);
		free(meshes);
		free(buffers);
		free(window);
		return NULL;
	}

	GLFWwindow* handle = glfwCreateWindow(
		(int)width,
		(int)height,
		title,
		NULL,
		NULL);

	if (handle == NULL)
	{
		free(pipelines);
		free(shaders);
		free(framebuffers);
		free(images);
		free(meshes);
		free(buffers);
		free(window);
		return NULL;
	}

	if (api == VULKAN_GRAPHICS_API)
	{
		// TODO: implement Vulkan functions
		return NULL;
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		glfwMakeContextCurrent(
			handle);

		if (gladLoadGL() == 0)
		{
			glfwDestroyWindow(handle);
			free(pipelines);
			free(shaders);
			free(framebuffers);
			free(images);
			free(meshes);
			free(buffers);
			free(window);
			return NULL;
		}

		GLint maxImageSize;

		glGetIntegerv(
			GL_MAX_TEXTURE_SIZE,
			&maxImageSize);

		window->maxImageSize =
			(size_t)maxImageSize;

		assertOpenGL();

		window->updateFunction =
			updateFunction;
		window->updateArgument =
			updateArgument;
		window->beginRecordFunction =
			beginGlCommandRecord;
		window->endRecordFunction =
			endGlCommandRecord;
		window->createBufferFunction =
			createGlBuffer;
		window->destroyBufferFunction =
			destroyGlBuffer;
		window->setBufferDataFunction =
			setGlBufferData;
		window->createMeshFunction =
			createGlMesh;
		window->destroyMeshFunction =
			destroyGlMesh;
		window->drawMeshFunction =
			drawGlMeshCommand;
		window->createImageFunction =
			createGlImage;
		window->destroyImageFunction =
			destroyGlImage;
		window->setImageDataFunction =
			setGlImageData;
		window->generateMipmapFunction =
			generateGlMipmap;
		window->getImageHandleFunction =
			getGlImageHandle;
		// TODO:
		window->createFramebufferFunction = NULL;
		window->destroyFramebufferFunction = NULL;
		window->createShaderFunction =
			createGlShader;
		window->destroyShaderFunction =
			destroyGlShader;
		window->getShaderHandleFunction =
			getGlShaderHandle;
	}

	window->api = api;
	window->buffers = buffers;
	window->bufferCapacity = 1;
	window->bufferCount = 0;
	window->meshes = meshes;
	window->meshCapacity = 1;
	window->meshCount = 0;
	window->images = images;
	window->imageCapacity = 1;
	window->imageCount = 0;
	window->framebuffers = framebuffers;
	window->framebufferCapacity = 1;
	window->framebufferCount = 0;
	window->shaders = shaders;
	window->shaderCapacity = 1;
	window->shaderCount = 0;
	window->pipelines = pipelines;
	window->pipelineCapacity = 1;
	window->pipelineCount = 0;
	window->handle = handle;
	window->updateTime = 0.0;
	window->deltaTime = 0.0;
	window->recording = false;
	return window;
}
struct Window* createAnyWindow(
	size_t width,
	size_t height,
	const char* title,
	UpdateWindow updateFunction,
	void* updateArgument)
{
	assert(width != 0);
	assert(height != 0);
	assert(title != NULL);
	assert(updateFunction != NULL);
	assert(graphicsInitialized == true);

	struct Window* window = createWindow(
		VULKAN_GRAPHICS_API,
		width,
		height,
		title,
		updateFunction,
		updateArgument);

	if (window != NULL)
		return window;

	window = createWindow(
		OPENGL_GRAPHICS_API,
		width,
		height,
		title,
		updateFunction,
		updateArgument);

	if (window != NULL)
		return window;

	window = createWindow(
		OPENGL_ES_GRAPHICS_API,
		width,
		height,
		title,
		updateFunction,
		updateArgument);

	return window;
}
void destroyWindow(
	struct Window* window)
{
	if (window == NULL)
        return;
    
    glfwDestroyWindow(window->handle);

    DestroyBuffer destroyBufferFunction =
    	window->destroyBufferFunction;
	size_t bufferCount =
		window->bufferCount;
    struct Buffer** buffers =
    	window->buffers;

	for (size_t i = 0; i < bufferCount; i++)
	{
		struct Buffer* buffer = buffers[i];

		destroyBufferFunction(
			window,
			buffer->handle);
		free(buffer);
	}

	free(buffers);

	DestroyMesh destroyMeshFunction =
		window->destroyMeshFunction;
	size_t meshCount =
		window->meshCount;
	struct Mesh** meshes =
		window->meshes;

	for (size_t i = 0; i < meshCount; i++)
	{
		struct Mesh* mesh = meshes[i];

		destroyMeshFunction(
			window,
			mesh->handle);
		free(mesh);
	}

	free(meshes);

	DestroyImage destroyImageFunction =
		window->destroyImageFunction;
	size_t imageCount =
		window->imageCount;
	struct Image** images =
		window->images;

	for (size_t i = 0; i < imageCount; i++)
	{
		struct Image* image = images[i];

		destroyImageFunction(
			window,
			image->handle);
		free(image);
	}

	free(images);

	DestroyFramebuffer destroyFramebufferFunction =
		window->destroyFramebufferFunction;
	size_t framebufferCount =
		window->framebufferCount;
	struct Framebuffer** framebuffers =
		window->framebuffers;

	for (size_t i = 0; i < framebufferCount; i++)
	{
		struct Framebuffer* framebuffer = framebuffers[i];

		destroyFramebufferFunction(
			window,
			framebuffer->handle);
		free(framebuffer);
	}

	free(framebuffers);

	DestroyShader destroyShaderFunction =
		window->destroyShaderFunction;
	size_t shaderCount =
		window->shaderCount;
	struct Shader** shaders =
		window->shaders;

	for (size_t i = 0; i < shaderCount; i++)
	{
		struct Shader* shader = shaders[i];

		destroyShaderFunction(
			window,
			shader->handle);
		free(shaders);
	}

	free(shaders);

	size_t pipelineCount =
		window->pipelineCount;
	struct Pipeline** pipelines =
		window->pipelines;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		struct Pipeline* pipeline = pipelines[i];

		pipeline->destroyFunction(
			window,
			pipeline->handle);
		free(pipeline);
	}

	free(pipelines);
	free(window);
}

uint8_t getWindowGraphicsAPI(
	const struct Window* window)
{
	assert(window != NULL);
	return window->api;
}
size_t getWindowMaxImageSize(
	const struct Window* window)
{
	assert(window != NULL);
	return window->maxImageSize;
}
double getWindowUpdateTime(
	const struct Window* window)
{
	assert(window != NULL);
	return window->updateTime;
}
double getWindowDeltaTime(
	const struct Window* window)
{
	assert(window != NULL);
	return window->deltaTime;
}
void getWindowSize(
	const struct Window* window,
	size_t* _width,
	size_t* _height)
{
	assert(window != NULL);
	assert(_width != NULL);
	assert(_height != NULL);

	int width, height;

	glfwGetWindowSize(
		window->handle,
		&width,
		&height);

	*_width = (size_t)width;
	*_height = (size_t)height;
}
void getWindowPosition(
	const struct Window* window,
	size_t* _x,
	size_t* _y)
{
	assert(window != NULL);
	assert(_x != NULL);
	assert(_y != NULL);

	int x, y;

	glfwGetWindowPos(
		window->handle,
		&x,
		&y);

	*_x = (size_t)x;
	*_y = (size_t)y;
}
void getWindowFramebufferSize(
	const struct Window* window,
	size_t* _width,
	size_t* _height)
{
	assert(window != NULL);
	assert(_width != NULL);
	assert(_height != NULL);

	int width, height;

	glfwGetFramebufferSize(
		window->handle,
		&width,
		&height);

	*_width = (size_t)width;
	*_height = (size_t)height;
}

void makeWindowContextCurrent(
	struct Window* window)
{
	assert(window != NULL);

	assert(window->api == OPENGL_GRAPHICS_API ||
		window->api == OPENGL_ES_GRAPHICS_API);

	glfwMakeContextCurrent(
		window->handle);
}
void updateWindow(
	struct Window* window)
{
	assert(window != NULL);
	assert(window->recording == false);

	struct GLFWwindow* handle =
		window->handle;

	while (glfwWindowShouldClose(handle) == GLFW_FALSE)
	{
		glfwPollEvents();

		double time = glfwGetTime();
		window->deltaTime = time - window->updateTime;
		window->updateTime = time;

		window->updateFunction(
			window->updateArgument);
	}
}

void beginCommandRecord(
	struct Window* window)
{
	assert(window != NULL);
	assert(window->recording == false);

	window->recording = true;
	window->beginRecordFunction(window);
}
void endCommandRecord(
	struct Window* window)
{
	assert(window != NULL);
	assert(window->recording == true);

	window->recording = false;
	window->endRecordFunction(window);
}

struct Buffer* createBuffer(
	struct Window* window,
	uint8_t type,
	const void* data,
	size_t size,
	bool constant)
{
	assert(window != NULL);
	assert(size != 0);
	assert(window->recording == false);

	struct Buffer* buffer =
		malloc(sizeof(struct Buffer));

	if (buffer == NULL)
		return NULL;

	void* handle = window->createBufferFunction(
		window,
		type,
		data,
		size,
		constant);

	if (handle == NULL)
	{
		free(buffer);
		return NULL;
	}

	buffer->window = window;
	buffer->type = type;
	buffer->size = size;
	buffer->constant = constant;
	buffer->handle = handle;

	if (window->bufferCount == window->bufferCapacity)
	{
		size_t capacity =
			window->bufferCapacity * 2;
		struct Buffer** buffers = realloc(
			window->buffers,
			capacity * sizeof(struct Buffer*));

		if (buffers == NULL)
		{
			window->destroyBufferFunction(
				window,
				handle);

			free(buffer);
			return NULL;
		}

		window->buffers = buffers;
		window->bufferCapacity = capacity;
	}

	window->buffers[window->bufferCount] = buffer;
	window->bufferCount++;
	return buffer;
}
void destroyBuffer(
	struct Buffer* buffer)
{
	assert(buffer->window->recording == false);

	if (buffer == NULL)
		return;

	struct Window* window =
		buffer->window;
	size_t bufferCount =
		window->bufferCount;
	struct Buffer** buffers =
		window->buffers;

	for (size_t i = 0; i < bufferCount; i++)
	{
		if (buffer == buffers[i])
		{
			for (size_t j = i + 1; j < bufferCount; j++)
				buffers[j - 1] = buffers[j];

			window->bufferCount--;

			window->destroyBufferFunction(
				window,
				buffer->handle);
			free(buffer);
			return;
		}
	}

	abort();
}

struct Window* getBufferWindow(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	return buffer->window;
}
uint8_t getBufferType(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	return buffer->type;
}
size_t getBufferSize(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	return buffer->size;
}
bool getBufferConstant(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	return buffer->constant;
}

void setBufferData(
	struct Buffer* buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	assert(buffer != NULL);
	assert(data != NULL);
	assert(size != 0);
	assert(buffer->constant == false);
	assert(size + offset <= buffer->size);
	assert(buffer->window->recording == false);

	buffer->window->setBufferDataFunction(
		buffer,
		data,
		size,
		offset);
}

struct Mesh* createMesh(
	struct Window* window,
	uint8_t drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer)
{
	assert(window != NULL);
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);
	assert(window == vertexBuffer->window);
	assert(window == indexBuffer->window);
	assert(vertexBuffer->type == VERTEX_BUFFER_TYPE);
	assert(indexBuffer->type == INDEX_BUFFER_TYPE);
	assert(window->recording == false);

	assert(drawIndex == UINT32_DRAW_INDEX ?
		indexCount * sizeof(uint32_t) <= indexBuffer->size :
		indexCount * sizeof(uint16_t) <= indexBuffer->size);

	struct Mesh* mesh =
		malloc(sizeof(struct Mesh));

	if (mesh == NULL)
		return NULL;

	void* handle = window->createMeshFunction(window);

	if (handle == NULL)
	{
		free(mesh);
		return NULL;
	}

	mesh->window = window;
	mesh->drawIndex = drawIndex;
	mesh->indexCount = indexCount;
	mesh->vertexBuffer = vertexBuffer;
	mesh->indexBuffer = indexBuffer;
	mesh->handle = handle;

	if (window->meshCount == window->meshCapacity)
	{
		size_t capacity =
			window->meshCapacity * 2;
		struct Mesh** meshes = realloc(
			window->meshes,
			capacity * sizeof(struct Mesh*));

		if (meshes == NULL)
		{
			window->destroyMeshFunction(
				window,
				handle);

			free(mesh);
			return NULL;
		}

		window->meshes = meshes;
		window->meshCapacity = capacity;
	}

	window->meshes[window->meshCount] = mesh;
	window->meshCount++;
	return mesh;
}
void destroyMesh(
	struct Mesh* mesh)
{
	assert(mesh->window->recording == false);

	if (mesh == NULL)
		return;

	struct Window* window =
		mesh->window;
	size_t meshCount =
		window->meshCount;
	struct Mesh** meshes =
		window->meshes;

	for (size_t i = 0; i < meshCount; i++)
	{
		if (mesh == meshes[i])
		{
			for (size_t j = i + 1; j < meshCount; j++)
				meshes[j - 1] = meshes[j];

			window->meshCount--;

			window->destroyMeshFunction(
				window,
				mesh->handle);
			free(mesh);
			return;
		}
	}

	abort();
}

struct Window* getMeshWindow(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->window;
}
uint8_t getMeshDrawIndex(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->drawIndex;
}
size_t getMeshIndexCount(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->indexCount;
}
void setMeshIndexCount(
	struct Mesh* mesh,
	size_t indexCount)
{
	assert(mesh != NULL);
	assert(mesh->window->recording == false);

	assert(mesh->drawIndex == UINT32_DRAW_INDEX ?
		indexCount * sizeof(uint32_t) <= mesh->indexBuffer->size :
		indexCount * sizeof(uint16_t) <= mesh->indexBuffer->size);

	mesh->indexCount = indexCount;
}

struct Buffer* getMeshVertexBuffer(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->vertexBuffer;
}
void setMeshVertexBuffer(
	struct Mesh* mesh,
	struct Buffer* vertexBuffer)
{
	assert(mesh != NULL);
	assert(vertexBuffer != NULL);
	assert(mesh->window == vertexBuffer->window);
	assert(vertexBuffer->type == VERTEX_BUFFER_TYPE);
	assert(mesh->window->recording == false);
	mesh->vertexBuffer = vertexBuffer;
}

struct Buffer* getMeshIndexBuffer(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->indexBuffer;
}
void setMeshIndexBuffer(
	struct Mesh* mesh,
	uint8_t drawIndex,
	size_t indexCount,
	struct Buffer* indexBuffer)
{
	assert(mesh != NULL);
	assert(indexCount != 0);
	assert(indexBuffer != NULL);
	assert(mesh->window == indexBuffer->window);
	assert(indexBuffer->type == INDEX_BUFFER_TYPE);
	assert(mesh->window->recording == false);

	assert(drawIndex == UINT32_DRAW_INDEX ?
		indexCount * sizeof(uint32_t) <= indexBuffer->size :
		indexCount * sizeof(uint16_t) <= indexBuffer->size);

	mesh->drawIndex = drawIndex;
	mesh->indexCount = indexCount;
	mesh->indexBuffer = indexBuffer;
}

void getMeshBuffers(
	const struct Mesh* mesh,
	struct Buffer** vertexBuffer,
	struct Buffer** indexBuffer)
{
	assert(mesh != NULL);
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);

	*vertexBuffer = mesh->vertexBuffer;
	*indexBuffer = mesh->indexBuffer;
}
void setMeshBuffers(
	struct Mesh* mesh,
	uint8_t drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer)
{
	assert(mesh != NULL);
	assert(indexCount != 0);
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);
	assert(mesh->window == vertexBuffer->window);
	assert(mesh->window == indexBuffer->window);
	assert(vertexBuffer->type == VERTEX_BUFFER_TYPE);
	assert(indexBuffer->type == INDEX_BUFFER_TYPE);
	assert(mesh->window->recording == false);

	assert(drawIndex == UINT32_DRAW_INDEX ?
		indexCount * sizeof(uint32_t) <= indexBuffer->size :
		indexCount * sizeof(uint16_t) <= indexBuffer->size);

	mesh->drawIndex = drawIndex;
	mesh->indexCount = indexCount;
	mesh->vertexBuffer = vertexBuffer;
	mesh->indexBuffer = indexBuffer;
}

void drawMeshCommand(
	struct Mesh* mesh,
	struct Pipeline* pipeline)
{
	assert(mesh != NULL);
	assert(pipeline != NULL);
	assert(mesh->window == pipeline->window);
	assert(mesh->window->recording == true);

	mesh->window->drawMeshFunction(
		mesh,
		pipeline);
}

struct Image* createImage(
	struct Window* window,
	uint8_t type,
	uint8_t format,
	size_t width,
	size_t height,
	size_t depth,
	const void* pixels,
	bool mipmap)
{
	assert(window != NULL);
	assert(width != 0);
	assert(height != 0);
	assert(depth != 0);
	assert(window->recording == false);

	size_t maxImageSize =
		window->maxImageSize;

	if (width > maxImageSize ||
		height > maxImageSize ||
		depth > maxImageSize)
	{
		return NULL;
	}

	struct Image* image =
		malloc(sizeof(struct Image));

	if (image == NULL)
		return NULL;

	void* handle = window->createImageFunction(
		window,
		type,
		format,
		width,
		height,
		depth,
		pixels,
		mipmap);

	if (handle == NULL)
	{
		free(image);
		return NULL;
	}

	image->window = window;
	image->type = type;
	image->format = format;
	image->width = width;
	image->height = height;
	image->depth = depth;
	image->mipmap = mipmap;
	image->handle = handle;

	if (window->imageCount == window->imageCapacity)
	{
		size_t capacity =
			window->imageCapacity * 2;
		struct Image** images = realloc(
			window->images,
			capacity * sizeof(struct Image*));

		if (images == NULL)
		{
			window->destroyImageFunction(
				window,
				handle);

			free(image);
			return NULL;
		}

		window->images = images;
		window->imageCapacity = capacity;
	}

	window->images[window->imageCount] = image;
	window->imageCount++;
	return image;
}
void destroyImage(
	struct Image* image)
{
	assert(image->window->recording == false);

	if (image == NULL)
		return;

	struct Window* window =
		image->window;
	size_t imageCount =
		window->imageCount;
	struct Image** images =
		window->images;

	for (size_t i = 0; i < imageCount; i++)
	{
		if (image == images[i])
		{
			for (size_t j = i + 1; j < imageCount; j++)
				images[j - 1] = images[j];

			window->imageCount--;

			window->destroyImageFunction(
				window,
				image->handle);
			free(image);
			return;
		}
	}

	abort();
}

void setImageData(
	struct Image* image,
	const void* data,
	size_t width,
	size_t height,
	size_t depth,
	size_t widthOffset,
	size_t heightOffset,
	size_t depthOffset,
	size_t mipmapLevel)
{
	assert(image != NULL);
	assert(width != 0);
	assert(height != 0);
	assert(depth != 0);
	assert(width + widthOffset <= image->width);
	assert(height + heightOffset <= image->width);
	assert(depth + depthOffset <= image->width);
	assert(image->window->recording == false);

	// TODO: check for static image in Vulkan API

	image->window->setImageDataFunction(
		image,
		data,
		width,
		height,
		depth,
		widthOffset,
		heightOffset,
		depthOffset,
		mipmapLevel);
}
void generateMipmap(
	struct Image* image)
{
	assert(image != NULL);
	assert(image->mipmap == true);
	assert(image->window->recording == false);
	image->window->generateMipmapFunction(image);
}

struct Window* getImageWindow(
	const struct Image* image)
{
	assert(image != NULL);
	return image->window;
}
uint8_t getImageType(
	const struct Image* image)
{
	assert(image != NULL);
	return image->type;
}
uint8_t getImageFormat(
	const struct Image* image)
{
	assert(image != NULL);
	return image->format;
}
size_t getImageWidth(
	const struct Image* image)
{
	assert(image != NULL);
	return image->width;
}
size_t getImageHeight(
	const struct Image* image)
{
	assert(image != NULL);
	return image->height;
}
size_t getImageDepth(
	const struct Image* image)
{
	assert(image != NULL);
	return image->depth;
}
bool getImageMipmap(
	const struct Image* image)
{
	assert(image != NULL);
	return image->mipmap;
}
const void* getImageHandle(
	const struct Image* image)
{
	assert(image != NULL);
	return image->window->getImageHandleFunction(image);
}

struct Shader* createShader(
	struct Window* window,
	uint8_t type,
	const void* code,
	size_t size)
{
	assert(window != NULL);
	assert(code != NULL);
	assert(window->recording == false);

	struct Shader* shader =
		malloc(sizeof(struct Shader));

	if (shader == NULL)
		return NULL;

	void* handle = window->createShaderFunction(
		window,
		type,
		code,
		size);

	if (handle == NULL)
	{
		free(shader);
		return NULL;
	}

	shader->window = window;
	shader->type = type;
	shader->handle = handle;

	if (window->shaderCount == window->shaderCapacity)
	{
		size_t capacity =
			window->shaderCapacity * 2;
		struct Shader** shaders = realloc(
			window->shaders,
			capacity * sizeof(struct Shader*));

		if (shaders == NULL)
		{
			window->destroyShaderFunction(
				window,
				handle);

			free(shader);
			return NULL;
		}

		window->shaders = shaders;
		window->shaderCapacity = capacity;
	}

	window->shaders[window->shaderCount] = shader;
	window->shaderCount++;
	return shader;
}
struct Shader* readShaderFromFile(
	struct Window* window,
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

	char* code = malloc(
		(fileSize + 1) * sizeof(char));

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

	struct Shader* shader = createShader(
		window,
		type,
		code,
		fileSize);

	free(code);
	return shader;
}
void destroyShader(
	struct Shader* shader)
{
	assert(shader->window->recording == false);

	if (shader == NULL)
		return;

	struct Window* window =
		shader->window;
	size_t shaderCount =
		window->shaderCount;
	struct Shader** shaders =
		window->shaders;

	for (size_t i = 0; i < shaderCount; i++)
	{
		if (shader == shaders[i])
		{
			for (size_t j = i + 1; j < shaderCount; j++)
				shaders[j - 1] = shaders[j];

			window->shaderCount--;

			window->destroyShaderFunction(
				window,
				shader->handle);
			free(shader);
			return;
		}
	}

	abort();
}

struct Window* getShaderWindow(
	const struct Shader* shader)
{
	assert(shader != NULL);
	return shader->window;
}
uint8_t getShaderType(
	const struct Shader* shader)
{
	assert(shader != NULL);
	return shader->type;
}
const void* getShaderHandle(
	const struct Shader* shader)
{
	assert(shader != NULL);
	return shader->window->getShaderHandleFunction(shader);
}

struct Pipeline* createPipeline(
	struct Window* window,
	uint8_t drawMode,
	DestroyPipeline destroyFunction,
	BindPipelineCommand bindFunction,
	SetUniformsCommand setUniformsFunction,
	void* handle)
{
	assert(window != NULL);
	assert(destroyFunction != NULL);
	assert(bindFunction != NULL);
	assert(setUniformsFunction != NULL);
	assert(handle != NULL);
	assert(window->recording == false);

	struct Pipeline* pipeline =
		malloc(sizeof(struct Pipeline));

	if (pipeline == NULL)
		return NULL;

	pipeline->window = window;
	pipeline->drawMode = drawMode;
	pipeline->destroyFunction = destroyFunction;
	pipeline->bindFunction = bindFunction;
	pipeline->setUniformsFunction = setUniformsFunction;
	pipeline->handle = handle;

	if (window->pipelineCount == window->pipelineCapacity)
	{
		size_t capacity =
			window->pipelineCapacity * 2;
		struct Pipeline** pipelines = realloc(
			window->pipelines,
			capacity * sizeof(struct Pipeline*));

		if (pipelines == NULL)
		{
			destroyFunction(
				window,
				handle);

			free(pipeline);
			return NULL;
		}

		window->pipelines = pipelines;
		window->pipelineCapacity = capacity;
	}

	window->pipelines[window->pipelineCount] = pipeline;
	window->pipelineCount++;
	return pipeline;
}
void destroyPipeline(
	struct Pipeline* pipeline)
{
	assert(pipeline->window->recording == false);

	if (pipeline == NULL)
		return;

	struct Window* window =
		pipeline->window;
	size_t pipelineCount =
		window->pipelineCount;
	struct Pipeline** pipelines =
		window->pipelines;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		if (pipeline == pipelines[i])
		{
			for (size_t j = i + 1; j < pipelineCount; j++)
				pipelines[j - 1] = pipelines[j];

			window->pipelineCount--;

			pipeline->destroyFunction(
				window,
				pipeline->handle);
			free(pipeline);
			return;
		}
	}

	abort();
}

struct Window* getPipelineWindow(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	return pipeline->window;
}
uint8_t getPipelineDrawMode(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	return pipeline->drawMode;
}
void* getPipelineHandle(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	return pipeline->handle;
}

void bindPipelineCommand(
	struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->recording == true);
	pipeline->bindFunction(pipeline);
}
