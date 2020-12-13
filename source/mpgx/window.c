#include "mpgx/window.h"
#include "mpgx/shader.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define OPENGL_SHADER_HEADER "#version 330 core\n\n#define highp \n#define mediump \n#define lowp \n"
#define OPENGL_ES_SHADER_HEADER "#version 300 es\n"

struct Window
{
	enum GraphicsAPI api;
	struct GLFWwindow* handle;
	double updateTime;
	double deltaTime;
};

struct GlBuffer
{
	GLenum type;
	GLuint handle;
};
struct Buffer
{
	struct Window* window;
	enum BufferType type;
	size_t size;
	bool constant;
	void* handle;
};

struct GlMesh
{
	GLenum drawIndex;
	GLuint handle;
};
struct Mesh
{
	struct Window* window;
	enum DrawIndex drawIndex;
	size_t indexCount;
	struct Buffer* vertexBuffer;
	struct Buffer* indexBuffer;
	void* handle;
};

struct GlImage
{
	GLenum type;
	GLuint handle;
};
struct Image
{
	struct Window* window;
	void* handle;
};

struct GlShader
{
	GLuint handle;
};
struct Shader
{
	struct Window* window;
	enum ShaderStage stage;
	void* handle;
};

struct GlUniform
{
	enum UniformType type;
	GLint location;
};
struct GlPipeline
{
	struct GlUniform* uniforms;
	GLuint handle;
};
struct Pipeline
{
	struct Window* window;
	enum DrawMode drawMode;
	size_t uniformCount;
	void* handle;
};

static bool graphicsInitialized = false;

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

	int result = glfwInit();

	if(result == GLFW_FALSE)
		return false;

	glfwSetErrorCallback(
		glfwErrorCallback);

	graphicsInitialized = true;
	return true;
}
void terminateGraphics()
{
	if (graphicsInitialized == false)
		return;

	glfwTerminate();
	graphicsInitialized = false;
}
bool getGraphicsInitialized()
{
	return graphicsInitialized;
}

struct Window* createWindow(
	enum GraphicsAPI api,
	size_t width,
	size_t height,
	const char* title)
{
	assert(width != 0);
	assert(height != 0);
	assert(title != NULL);

	struct Window* window =
		malloc(sizeof(struct Window));

	if (window == NULL)
		return NULL;

	window->api = api;

	window->updateTime = 0.0;
	window->deltaTime = 0.0;

	glfwDefaultWindowHints();

	if (api == OPENGL_GRAPHICS_API)
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

	window->handle = glfwCreateWindow(
		(int)width,
		(int)height,
		title,
		NULL,
		NULL);

	if (window->handle == NULL)
	{
		free(window);
		return NULL;
	}

	glfwMakeContextCurrent(
		window->handle);

	if (gladLoadGL() == 0)
	{
		glfwDestroyWindow(window->handle);
		free(window);
		return NULL;
	}

	return window;
}
void destroyWindow(
	struct Window* window)
{
	if (window == NULL)
        return;
    
    glfwDestroyWindow(window->handle);
	free(window);
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

void startWindowUpdate(
	struct Window* window,
	WindowRender renderFunction,
	void* functionArgument)
{
	assert(window != NULL);
	assert(renderFunction != NULL);

	struct GLFWwindow* handle = window->handle;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		while (glfwWindowShouldClose(handle) == GLFW_FALSE)
		{
			glfwPollEvents();

			double time = glfwGetTime();
			window->deltaTime = time - window->updateTime;
			window->updateTime = time;

			renderFunction(functionArgument);
			glfwSwapBuffers(handle);
		}
	}
	else
	{
		abort();
	}
}

static struct GlBuffer* createGlBuffer(
	struct Window* window,
	enum BufferType _type,
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
		GL_DYNAMIC_DRAW :
		GL_STATIC_DRAW;

	glBindBuffer(
		type,
		handle);
	glBufferData(
		type,
		(GLsizeiptr)(size),
		data,
		usage);
	glBindBuffer(
		type,
		GL_ZERO);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	buffer->type = type;
	buffer->handle = handle;
	return buffer;
}
struct Buffer* createBuffer(
	struct Window* window,
	enum BufferType type,
	const void* data,
	size_t size,
	bool constant)
{
	assert(window != NULL);
	assert(size != 0);

	struct Buffer* buffer =
		malloc(sizeof(struct Buffer));

	if (buffer == NULL)
		return NULL;

	enum GraphicsAPI api =
		window->api;

	void* handle;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlBuffer(
			window,
			type,
			data,
			size,
			constant);
	}
	else
	{
		abort();
	}
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
	return buffer;
}

static void destroyGlBuffer(
	struct Window* window,
	struct GlBuffer* buffer)
{
	glfwMakeContextCurrent(
		window->handle);

	glDeleteBuffers(
		GL_ONE,
		&buffer->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(buffer);
}
void destroyBuffer(
	struct Buffer* buffer)
{
	if (buffer == NULL)
		return;

	struct Window* window = buffer->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		destroyGlBuffer(
			window,
			(struct GlBuffer*)buffer);
	}
	else
	{
		abort();
	}

	free(buffer);
}

struct Window* getBufferWindow(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	return buffer->window;
}
enum BufferType getBufferType(
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

void setGlBufferData(
	struct Window* window,
	struct GlBuffer* buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	glfwMakeContextCurrent(
		window->handle);

	glBindBuffer(
		buffer->type,
		buffer->handle);
	glBufferSubData(
		buffer->type,
		(GLintptr)offset,
		(GLsizeiptr)size,
		data);
	glBindBuffer(
		buffer->type,
		GL_ZERO);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
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

	struct Window* window = buffer->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		setGlBufferData(
			window,
			(struct GlBuffer*)buffer->handle,
			data,
			size,
			offset);
	}
	else
	{
		abort();
	}
}

static struct GlMesh* createGlMesh(
	struct Window* window,
	enum DrawIndex _drawIndex,
	struct GlBuffer* vertexBuffer,
	struct GlBuffer* indexBuffer)
{
	struct GlMesh* mesh = malloc(
		sizeof(struct GlMesh));

	if (mesh == NULL)
		return NULL;

	GLenum drawIndex;

	if (_drawIndex == UINT16_DRAW_MODE)
	{
		drawIndex = GL_UNSIGNED_SHORT;
	}
	else if (_drawIndex == UINT32_DRAW_MODE)
	{
		drawIndex = GL_UNSIGNED_INT;
	}
	else
	{
		free(mesh);
		return NULL;
	}

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = GL_ZERO;

	glGenVertexArrays(
		GL_ONE,
		&handle);

	glBindVertexArray(
		handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		vertexBuffer->handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		indexBuffer->handle);

	glBindVertexArray(
		GL_ZERO);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		GL_ZERO);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		GL_ZERO);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	mesh->drawIndex = drawIndex;
	mesh->handle = handle;
	return mesh;
}
struct Mesh* createMesh(
	struct Window* window,
	enum DrawIndex drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer)
{
	assert(window != NULL);
	assert(indexCount != 0);
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);
	assert(window == vertexBuffer->window);
	assert(window == indexBuffer->window);
	assert(vertexBuffer->type == VERTEX_BUFFER_TYPE);
	assert(indexBuffer->type == INDEX_BUFFER_TYPE);

	assert(drawIndex == UINT16_DRAW_MODE ?
		indexCount * sizeof(uint16_t) <= indexBuffer->size :
		indexCount * sizeof(uint32_t) <= indexBuffer->size);

	struct Mesh* mesh =
		malloc(sizeof(struct Mesh));

	if (mesh == NULL)
		return NULL;

	enum GraphicsAPI api =
		window->api;

	void* handle;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlMesh(
			window,
			drawIndex,
			(struct GlBuffer*)vertexBuffer,
			(struct GlBuffer*)indexBuffer);
	}
	else
	{
		abort();
	}

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
	return mesh;
}

static void destroyGlMesh(
	struct Window* window,
	struct GlMesh* mesh)
{
	glfwMakeContextCurrent(
		window->handle);

	glDeleteVertexArrays(
		GL_ONE,
		&mesh->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(mesh);
}
void destroyMesh(
	struct Mesh* mesh)
{
	if (mesh == NULL)
		return;

	struct Window* window = mesh->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		destroyGlMesh(
			window,
			(struct GlMesh*)mesh->handle);
	}
	else
	{
		abort();
	}

	free(mesh);
}

struct Window* getMeshWindow(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->window;
}

size_t getMeshIndexCount(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->indexCount;
}
void setMeshIndexCount(
	struct Mesh* mesh,
	size_t count)
{
	assert(mesh != NULL);
	assert(count != 0);

	assert(mesh->drawIndex == UINT16_DRAW_MODE ?
		count * sizeof(uint16_t) <= mesh->indexBuffer->size :
		count * sizeof(uint32_t) <= mesh->indexBuffer->size);

	mesh->indexCount = count;
}

struct Buffer* getMeshVertexBuffer(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->vertexBuffer;
}

void setGlMeshVertexBuffer(
	struct Window* window,
	struct GlMesh* mesh,
	struct GlBuffer* buffer)
{
	glfwMakeContextCurrent(
		window->handle);

	glBindVertexArray(
		mesh->handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		buffer->handle);

	glBindVertexArray(
		GL_ZERO);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		GL_ZERO);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
}
void setMeshVertexBuffer(
	struct Mesh* mesh,
	struct Buffer* buffer)
{
	assert(mesh != NULL);
	assert(buffer != NULL);
	assert(mesh->window == buffer->window);
	assert(buffer->type == VERTEX_BUFFER_TYPE);

	struct Window* window = mesh->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		setGlMeshVertexBuffer(
			window,
			(struct GlMesh*)mesh->handle,
			(struct GlBuffer*)mesh->vertexBuffer);
	}
	else
	{
		abort();
	}
}

struct Buffer* getMeshIndexBuffer(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->indexBuffer;
}

void setGlMeshIndexBuffer(
	struct Window* window,
	struct GlMesh* mesh,
	struct GlBuffer* buffer)
{
	glfwMakeContextCurrent(
		window->handle);

	glBindVertexArray(
		mesh->handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		buffer->handle);

	glBindVertexArray(
		GL_ZERO);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		GL_ZERO);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
}
void setMeshIndexBuffer(
	struct Mesh* mesh,
	struct Buffer* buffer)
{
	assert(mesh != NULL);
	assert(buffer != NULL);
	assert(mesh->window == buffer->window);
	assert(buffer->type == INDEX_BUFFER_TYPE);

	struct Window* window = mesh->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		setGlMeshVertexBuffer(
			window,
			(struct GlMesh*)mesh->handle,
			(struct GlBuffer*)mesh->indexBuffer);
	}
	else
	{
		abort();
	}
}

static GLuint glDrawMode;

void drawGlMeshCommand(
	size_t indexCount,
	struct GlMesh* mesh)
{
	glBindVertexArray(
		mesh->handle);
	glDrawElements(
		glDrawMode,
		(GLsizei)indexCount,
		mesh->drawIndex,
		NULL);
	glBindVertexArray(
		GL_ZERO);
}
void drawMeshCommand(
	struct Mesh* mesh)
{
	assert(mesh != NULL);

	struct Window* window = mesh->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		drawGlMeshCommand(
			mesh->indexCount,
			(struct GlMesh*)mesh->handle);
	}
	else
	{
		abort();
	}
}

static struct GlImage* createGlImage(
	struct Window* window,
	enum ImageType _type,
	enum ImageFormat _format,
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

	if (_type == IMAGE_1D_TYPE)
	{
		type = GL_TEXTURE_1D;
	}
	else if (_type == IMAGE_2D_TYPE)
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
	GLenum dataFormat;
	GLenum dataType;

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

	if (_type == IMAGE_1D_TYPE)
	{
		glTexImage1D(
			type,
			0,
			format,
			(GLsizei)width,
			0,
			dataFormat,
			dataType,
			pixels);
	}
	else if (_type == IMAGE_2D_TYPE)
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

	if (mipmap)
		glGenerateMipmap(type);

	glBindTexture(
		type,
		GL_ZERO);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	image->type = type;
	image->handle = handle;
	return image;
}
struct Image* createImage(
	struct Window* window,
	enum ImageType type,
	enum ImageFormat format,
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

	struct Image* image =
		malloc(sizeof(struct Image));

	if (image == NULL)
		return NULL;

	enum GraphicsAPI api =
		window->api;

	void* handle;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlImage(
			window,
			type,
			format,
			width,
			height,
			depth,
			pixels,
			mipmap);
	}
	else
	{
		abort();
	}

	if (handle == NULL)
	{
		free(image);
		return NULL;
	}

	// TODO: set values
	image->window = window;
	image->handle = handle;
	return image;
}

static void destroyGlImage(
	struct Window* window,
	struct GlImage* image)
{
	glfwMakeContextCurrent(
		window->handle);

	glDeleteTextures(
		GL_ONE,
		&image->handle);

	free(image);
}
void destroyImage(
	struct Image* image)
{
	if (image == NULL)
		return;

	struct Window* window = image->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		destroyGlImage(
			window,
			(struct GlImage*)image->handle);
	}
	else
	{
		abort();
	}

	free(image);
}

struct Window* getImageWindow(
	const struct Image* image)
{
	assert(image != NULL);
	return image->window;
}

static struct GlShader* createGlShader(
	struct Window* window,
	enum ShaderStage _stage,
	const void* program,
	bool gles)
{
	struct GlShader* shader = malloc(
		sizeof(struct GlShader));

	if (shader == NULL)
		return NULL;

	GLenum stage;

	if (_stage == VERTEX_SHADER_STAGE)
	{
		stage = GL_VERTEX_SHADER;
	}
	else if (_stage == FRAGMENT_SHADER_STAGE)
	{
		stage = GL_FRAGMENT_SHADER;
	}
	else if (_stage == COMPUTE_SHADER_STAGE)
	{
		stage = GL_COMPUTE_SHADER;
	}
	else
	{
		free(shader);
		return NULL;
	}

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = glCreateShader(stage);

	const char* sources[2];

	if (gles == false)
		sources[0] = OPENGL_SHADER_HEADER;
	else
		sources[0] = OPENGL_ES_SHADER_HEADER;

	sources[1] = (const char*)program;

	glShaderSource(
		handle,
		2,
		sources,
		NULL);

	glCompileShader(handle);

	GLint compileResult;

	glGetShaderiv(
		handle,
		GL_COMPILE_STATUS,
		&compileResult);

	if (compileResult == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetShaderiv(
			handle,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char infoLog[length];

			glGetShaderInfoLog(
				handle,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		free(shader);
		glDeleteShader(handle);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	shader->handle = handle;
	return shader;
}
struct Shader* createShader(
	struct Window* window,
	enum ShaderStage stage,
	const void* program,
	size_t size)
{
	assert(window != NULL);
	assert(program != NULL);
	assert(size != 0);

	struct Shader* shader =
		malloc(sizeof(struct Shader));

	if (shader == NULL)
		return NULL;

	enum GraphicsAPI api =
		window->api;

	void* handle;

	if (api == OPENGL_GRAPHICS_API)
	{
		handle = createGlShader(
			window,
			stage,
			program,
			false);
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlShader(
			window,
			stage,
			program,
			true);
	}
	else
	{
		abort();
	}

	if (handle == NULL)
	{
		free(shader);
		return NULL;
	}

	shader->window = window;
	shader->stage = stage;
	shader->handle = handle;
	return shader;
}

static void destroyGlShader(
	struct Window* window,
	struct GlShader* shader)
{
	glfwMakeContextCurrent(
		window->handle);

	glDeleteShader(shader->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(shader);
}
void destroyShader(
	struct Shader* shader)
{
	if (shader == NULL)
		return;

	struct Window* window = shader->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		destroyGlShader(
			window,
			(struct GlShader*)shader->handle);
	}
	else
	{
		abort();
	}

	free(shader);
}

struct Window* getShaderWindow(
	const struct Shader* shader)
{
	assert(shader != NULL);
	return shader->window;
}
enum ShaderStage getShaderStage(
	const struct Shader* shader)
{
	assert(shader != NULL);
	return shader->stage;
}

struct GlPipeline* createGlPipeline(
	struct Window* window,
	const struct Shader** shaders,
	size_t shaderCount,
	const struct UniformData* _uniforms,
	size_t uniformCount)
{
	struct GlPipeline* pipeline = malloc(
		sizeof(struct GlPipeline));

	if (pipeline == NULL)
		return NULL;

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = glCreateProgram();

	for (size_t i = 0; i < shaderCount; i++)
	{
		const struct Shader* shader = shaders[i];

		struct GlShader* glShader =
			(struct GlShader*)shader->handle;

		glAttachShader(
			handle,
			glShader->handle);
	}

	glLinkProgram(handle);

	for (size_t i = 0; i < shaderCount; i++)
	{
		const struct Shader* shader = shaders[i];

		struct GlShader* glShader =
			(struct GlShader*)shader->handle;

		glDetachShader(
			handle,
			glShader->handle);
	}

	GLint linkResult;

	glGetProgramiv(
		handle,
		GL_LINK_STATUS,
		&linkResult);

	if (linkResult == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetProgramiv(
			handle,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char infoLog[length];

			glGetProgramInfoLog(
				handle,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		free(pipeline);
		glDeleteProgram(handle);
		return NULL;
	}

	struct GlUniform* uniforms = malloc(
		uniformCount * sizeof(struct GlUniform));

	if (uniforms == NULL)
	{
		free(pipeline);
		glDeleteProgram(handle);
		return NULL;
	}

	for (size_t i = 0; i < uniformCount; ++i)
	{
		struct UniformData data = _uniforms[i];

		GLint location = glGetUniformLocation(
			handle,
			data.name);

		if (location == -1)
		{
#ifndef NDEBUG
			printf("Failed to get '%s' uniform location\n",
				data.name);
#endif

			free(pipeline);
			free(uniforms);
			glDeleteProgram(handle);
			return NULL;
		}

		struct GlUniform uniform;
		uniform.type = data.type;
		uniform.location = location;
		uniforms[i] = uniform;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	pipeline->uniforms = uniforms;
	pipeline->handle = handle;
	return pipeline;
}
struct Pipeline* createPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	const struct Shader** shaders,
	size_t shaderCount,
	const struct UniformData* uniforms,
	size_t uniformCount)
{
	assert(window != NULL);
	assert(shaders != NULL);
	assert(shaderCount != 0);
	assert(uniforms != NULL);
	assert(uniformCount != 0);

#ifndef NDEBUG
	size_t uniformDataSize = 0;

	for (size_t i = 0; i < uniformCount; i++)
	{
		if (uniforms[i].type == VECTOR_4F_UNIFORM_TYPE)
			uniformDataSize += sizeof(float) * 4;
		else if (uniforms[i].type == MATRIX_4F_UNIFORM_TYPE)
			uniformDataSize += sizeof(float) * 16;
		else
			abort();
	}

	// Vulkan push-constant min size
	assert(uniformDataSize <= 128);
#endif

	struct Pipeline* pipeline =
		malloc(sizeof(struct Pipeline));

	if (pipeline == NULL)
		return NULL;

	enum GraphicsAPI api =
		window->api;

	void* handle;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlPipeline(
			window,
			shaders,
			shaderCount,
			uniforms,
			uniformCount);
	}
	else
	{
		abort();
	}

	if (handle == NULL)
	{
		free(pipeline);
		return NULL;
	}

	pipeline->window = window;
	pipeline->drawMode = drawMode;
	pipeline->uniformCount = uniformCount;
	pipeline->handle = handle;
	return pipeline;
}

void destroyGlPipeline(
	struct Window* window,
	struct GlPipeline* pipeline)
{
	glfwMakeContextCurrent(
		window->handle);

	glDeleteProgram(pipeline->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(pipeline->uniforms);
	free(pipeline);
}
void destroyPipeline(
	struct Pipeline* pipeline)
{
	if (pipeline == NULL)
		return;

	struct Window* window = pipeline->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		destroyGlPipeline(
			window,
			(struct GlPipeline*)pipeline->handle);
	}
	else
	{
		abort();
	}

	free(pipeline);
}

void setGlUniformCommand(
	struct Window* window,
	struct GlPipeline* pipeline,
	size_t index,
	const void* data)
{
	struct GlUniform uniform =
		pipeline->uniforms[index];

	if (uniform.type == VECTOR_4F_UNIFORM_TYPE)
	{
		glUniform4fv(
			uniform.location,
			GL_ONE,
			data);
	}
	else if (uniform.type == MATRIX_4F_UNIFORM_TYPE)
	{
		glUniformMatrix4fv(
			uniform.location,
			GL_ONE,
			GL_FALSE,
			data);
	}
	else
	{
		abort();
	}
}
void addUniformCommand(
	struct Pipeline* pipeline,
	size_t index,
	const void* data)
{
	assert(pipeline != NULL);
	assert(data != NULL);
	assert(index < pipeline->uniformCount);

	struct Window* window = pipeline->window;
	enum GraphicsAPI api = window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		setGlUniformCommand(
			window,
			(struct GlPipeline*)pipeline->handle,
			index,
			data);
	}
	else
	{
		abort();
	}
}

struct Shader* createColorVertexShader(
	struct Window* window)
{
	assert(window != NULL);

	enum GraphicsAPI api =
		window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return createShader(
			window,
			VERTEX_SHADER_STAGE,
			OPENGL_COLOR_VERTEX_SHADER,
			sizeof(OPENGL_COLOR_VERTEX_SHADER));
	}
	else
	{
		abort();
	}
}
struct Shader* createColorFragmentShader(
	struct Window* window)
{
	assert(window != NULL);

	enum GraphicsAPI api =
		window->api;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return createShader(
			window,
			FRAGMENT_SHADER_STAGE,
			OPENGL_COLOR_FRAGMENT_SHADER,
			sizeof(OPENGL_COLOR_FRAGMENT_SHADER));
	}
	else
	{
		abort();
	}
}
struct Pipeline* createColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	const struct Shader* vertexShader,
	const struct Shader* fragmentShader)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);

	const struct Shader* shaders[2] =
	{
		vertexShader,
		fragmentShader
	};

	struct UniformData uniforms[2];
	uniforms[0].name = "u_MVP";
	uniforms[0].type = MATRIX_4F_UNIFORM_TYPE;
	uniforms[1].name = "u_Color";
	uniforms[1].type = VECTOR_4F_UNIFORM_TYPE;

	return createPipeline(
		window,
		drawMode,
		shaders,
		2,
		uniforms,
		2);
}
