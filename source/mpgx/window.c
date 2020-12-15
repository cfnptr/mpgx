#include "mpgx/window.h"
#include "mpgx/vector.h"
#include "mpgx/matrix.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define OPENGL_SHADER_HEADER "#version 330 core\n\n#define highp \n#define mediump \n#define lowp \n"
#define OPENGL_ES_SHADER_HEADER "#version 300 es\n"

#define OPENGL_COLOR_VERTEX_SHADER                 \
"layout(location = 0) in highp vec3 v_Position;\n  \
uniform highp mat4 u_MVP;\n                        \
                                                   \
void main()\n                                      \
{\n                                                \
	gl_Position = u_MVP * vec4(v_Position, 1.0);\n \
}\n"


#define OPENGL_COLOR_FRAGMENT_SHADER \
"out highp vec4 o_Color;\n           \
uniform highp vec4 u_Color;\n        \
                                     \
void main()\n                        \
{\n                                  \
	o_Color = u_Color;\n             \
}\n"

typedef void(*BeginCommandRecord)(
	struct Window*);
typedef void(*EndCommandRecord)(
	struct Window*);

struct Window
{
	enum GraphicsAPI api;
	double updateTime;
	double deltaTime;
	bool recording;
	BeginCommandRecord beginRecordFunction;
	BeginCommandRecord endRecordFunction;
	struct GLFWwindow* handle;
};

typedef void(*DestroyBuffer)(
	struct Buffer*);
typedef void(*SetBufferData)(
	struct Buffer*,
	const void*,
	size_t,
	size_t);

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
	DestroyBuffer destroyFunction;
	SetBufferData setDataFunction;
	void* handle;
};

typedef void(*DestroyMesh)(
	struct Mesh*);
typedef void(*DrawMeshCommand)(
	struct Mesh*);

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
	DestroyMesh destroyFunction;
	DrawMeshCommand drawFunction;
	void* handle;
};

typedef void(*DestroyImage)(
	struct Image*);

struct GlImage
{
	GLenum type;
	GLuint handle;
};
struct Image
{
	struct Window* window;
	DestroyImage destroyFunction;
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

void beginGlCommandRecord(
	struct Window* window)
{
	glfwMakeContextCurrent(
		window->handle);
}
void endGlCommandRecord(
	struct Window* window)
{
	glfwSwapBuffers(
		window->handle);
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

		window->beginRecordFunction =
			beginGlCommandRecord;
		window->endRecordFunction =
			endGlCommandRecord;
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

		window->beginRecordFunction =
			beginGlCommandRecord;
		window->endRecordFunction =
			endGlCommandRecord;
	}
	else
	{
		abort();
	}

	GLFWwindow* handle = glfwCreateWindow(
		(int)width,
		(int)height,
		title,
		NULL,
		NULL);

	if (handle == NULL)
	{
		free(window);
		return NULL;
	}

	glfwMakeContextCurrent(
		handle);

	if (gladLoadGL() == 0)
	{
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	window->api = api;
	window->updateTime = 0.0;
	window->deltaTime = 0.0;
	window->recording = false;
	window->handle = handle;
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

	struct GLFWwindow* handle =
		window->handle;

	while (glfwWindowShouldClose(handle) == GLFW_FALSE)
	{
		glfwPollEvents();

		double time = glfwGetTime();
		window->deltaTime = time - window->updateTime;
		window->updateTime = time;

		renderFunction(functionArgument);
	}
}

void beginCommandRecord(
	struct Window* window)
{
	assert(window != NULL);
	assert(window->recording == false);

	window->recording = true;

	BeginCommandRecord beginFunction =
		window->beginRecordFunction;
	beginFunction(window);
}
void endCommandRecord(
	struct Window* window)
{
	assert(window != NULL);
	assert(window->recording == true);

	window->recording = false;

	BeginCommandRecord endFunction =
		window->endRecordFunction;
	endFunction(window);
}

inline static struct GlBuffer* createGlBuffer(
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

	glfwMakeContextCurrent(
		window->handle);

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
void destroyGlBuffer(
	struct Buffer* buffer)
{
	struct GlBuffer* glBuffer =
		(struct GlBuffer*)buffer->handle;

	glfwMakeContextCurrent(
		buffer->window->handle);

	glDeleteBuffers(
		GL_ONE,
		&glBuffer->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

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
	glBindBuffer(
		glBuffer->type,
		GL_ZERO);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
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

		buffer->destroyFunction = destroyGlBuffer;
		buffer->setDataFunction = setGlBufferData;
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
void destroyBuffer(
	struct Buffer* buffer)
{
	if (buffer == NULL)
		return;

	DestroyBuffer destroyFunction =
		buffer->destroyFunction;

	destroyFunction(buffer);
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

	SetBufferData setDataFunction =
		buffer->setDataFunction;

	setDataFunction(
		buffer,
		data,
		size,
		offset);
}

inline static struct GlMesh* createGlMesh(
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
void destroyGlMesh(
	struct Mesh* mesh)
{
	struct GlMesh* glMesh =
		(struct GlMesh*)mesh->handle;

	glfwMakeContextCurrent(
		mesh->window->handle);

	glDeleteVertexArrays(
		GL_ONE,
		&glMesh->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(glMesh);
}
static void drawGlMeshCommand(
	struct Mesh* mesh)
{
	struct GlMesh* glMesh =
		(struct GlMesh*)mesh->handle;
	struct GlBuffer* glVertexBuffer =
		(struct GlBuffer*)mesh->vertexBuffer;
	struct GlBuffer* glIndexBuffer =
		(struct GlBuffer*)mesh->indexBuffer;

	glfwMakeContextCurrent(
		mesh->window->handle);

	glBindVertexArray(
		glMesh->handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		glVertexBuffer->handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		glVertexBuffer->handle);

	// TODO: fix draw mode
	glDrawElements(
		window->drawMode,
		(GLsizei)mesh->indexCount,
		glMesh->drawIndex,
		NULL);

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

		mesh->destroyFunction = destroyGlMesh;
		mesh->drawFunction = drawGlMeshCommand;
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
void destroyMesh(
	struct Mesh* mesh)
{
	if (mesh == NULL)
		return;

	DestroyMesh destroyFunction =
		mesh->destroyFunction;
	destroyFunction(mesh);

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

void setMeshVertexBuffer(
	struct Mesh* mesh,
	struct Buffer* buffer)
{
	assert(mesh != NULL);
	assert(buffer != NULL);
	assert(mesh->window == buffer->window);
	assert(buffer->type == VERTEX_BUFFER_TYPE);
	mesh->vertexBuffer = buffer;
}

struct Buffer* getMeshIndexBuffer(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->indexBuffer;
}

void setMeshIndexBuffer(
	struct Mesh* mesh,
	struct Buffer* buffer)
{
	assert(mesh != NULL);
	assert(buffer != NULL);
	assert(mesh->window == buffer->window);
	assert(buffer->type == INDEX_BUFFER_TYPE);
	mesh->indexBuffer = buffer;
}

void drawMeshCommand(
	struct Mesh* mesh)
{
	assert(mesh != NULL);
	assert(mesh->window->recording == true);

	DrawMeshCommand drawFunction =
		mesh->drawFunction;
	drawFunction(mesh);
}

inline static struct GlImage* createGlImage(
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
void destroyGlImage(
	struct Image* image)
{
	struct GlImage* glImage =
		(struct GlImage*)image->handle;

	glfwMakeContextCurrent(
		image->window->handle);

	glDeleteTextures(
		GL_ONE,
		&glImage->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(glImage);
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

		image->destroyFunction = destroyGlImage;
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
void destroyImage(
	struct Image* image)
{
	if (image == NULL)
		return;

	DestroyImage destroyFunction =
		image->destroyFunction;
	destroyFunction(image);

	free(image);
}

struct Window* getImageWindow(
	const struct Image* image)
{
	assert(image != NULL);
	return image->window;
}

inline static GLuint createGlShader(
	GLenum stage,
	const char* source,
	bool gles)
{
	GLuint shader = glCreateShader(stage);

	const char* sources[2];

	if (gles == false)
		sources[0] = OPENGL_SHADER_HEADER;
	else
		sources[0] = OPENGL_ES_SHADER_HEADER;

	sources[1] = source;

	glShaderSource(
		shader,
		2,
		sources,
		NULL);

	glCompileShader(shader);

	GLint result;

	glGetShaderiv(
		shader,
		GL_COMPILE_STATUS,
		&result);

	if (result == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetShaderiv(
			shader,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char infoLog[length];

			glGetShaderInfoLog(
				shader,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		glDeleteShader(shader);
		return GL_ZERO;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	return shader;
}
inline static GLuint createGlProgram(
	const GLenum* shaderStages,
	const char** shaderSources,
	size_t shaderCount,
	bool gles)
{
	GLuint program = glCreateProgram();

	GLuint shaders[shaderCount];

	for (size_t i = 0; i < shaderCount; i++)
	{
		shaders[i] = createGlShader(
			shaderStages[i],
			shaderSources[i],
			gles);
	}

	for (size_t i = 0; i < shaderCount; i++)
	{
		glAttachShader(
			program,
			shaders[i]);
	}

	glLinkProgram(program);

	for (size_t i = 0; i < shaderCount; i++)
	{
		glDetachShader(
			program,
			shaders[i]);
	}

	for (size_t i = 0; i < shaderCount; i++)
		glDeleteShader(shaders[i]);

	GLint result;

	glGetProgramiv(
		program,
		GL_LINK_STATUS,
		&result);

	if (result == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetProgramiv(
			program,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char infoLog[length];

			glGetProgramInfoLog(
				program,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		glDeleteProgram(program);
		return GL_ZERO;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	return program;
}

struct GlColorPipeline
{
	struct Matrix4F mvp;
	struct Vector4F color;
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
};

inline static struct GlColorPipeline* createGlColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	const void* vertexShader,
	const void* fragmentShader,
	bool gles)
{
	struct GlColorPipeline* pipeline = malloc(
		sizeof(struct GlColorPipeline));

	if (pipeline == NULL)
		return NULL;

	GLenum stages[2] =
	{
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
	};
	const char* shaders[2] =
	{
		(const char*)vertexShader,
		(const char*)fragmentShader,
	};

	glfwMakeContextCurrent(
		window->handle);

	GLuint handle = createGlProgram(
		stages,
		shaders,
		2,
		gles);

	if (handle == GL_ZERO)
	{
		free(pipeline);
		return NULL;
	}

	GLint mvpLocation = glGetUniformLocation(
		handle,
		"u_MVP");

	if (mvpLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_MVP' location\n");
#endif

		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint colorLocation = glGetUniformLocation(
		handle,
		"u_Color");

	if (colorLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Color' location\n");
#endif

		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	pipeline->handle = handle;
	pipeline->mvpLocation = mvpLocation;
	pipeline->colorLocation = colorLocation;
	return pipeline;
}

void destroyPipeline(
	struct Pipeline* pipeline)
{
	if (pipeline == NULL)
		return;

	assert(pipeline->destroyFunction != NULL);

	DestroyPipeline destroyFunction =
		pipeline->destroyFunction;

	destroyFunction(pipeline);
	free(pipeline);
}
void destroyGlColorPipeline(
	struct Pipeline* pipeline)
{
	struct GlColorPipeline* glPipeline =
		(struct GlColorPipeline*)pipeline->handle;

	glfwMakeContextCurrent(
		pipeline->window->handle);

	glDeleteProgram(glPipeline->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(glPipeline);
}
void bindGlColorPipelineCommand(
	struct Pipeline* pipeline)
{
	struct GlColorPipeline* glPipeline =
		(struct GlColorPipeline*)pipeline->handle;

	glUseProgram(glPipeline->handle);
}
void flushGlColorPipelineCommand(
	struct Pipeline* pipeline)
{
	// TODO: SET UNIFORMS
}

void bindPipelineCommand(
	struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->bindFunction != NULL);
	assert(pipeline->window != NULL);
	assert(pipeline->window->recording == true);

	BindPipelineCommand bindFunction =
		pipeline->bindFunction;
	bindFunction(pipeline);
}
void flushPipelineCommand(
	struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->flushFunction != NULL);
	assert(pipeline->window != NULL);
	assert(pipeline->window->recording == true);

	FlushPipelineCommand flushFunction =
		pipeline->flushFunction;
	flushFunction(pipeline);
}

struct Pipeline* createColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	const void* vertexShader,
	size_t vertexShaderSize,
	const void* fragmentShader,
	size_t fragmentShaderSize)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(vertexShaderSize != 0);
	assert(fragmentShader != NULL);
	assert(fragmentShaderSize != 0);

	struct Pipeline* pipeline =
		malloc(sizeof(struct Pipeline));

	if (pipeline == NULL)
		return NULL;

	enum GraphicsAPI api =
		window->api;

	void* handle;

	if (api == OPENGL_GRAPHICS_API)
	{
		handle = createGlColorPipeline(
			window,
			drawMode,
			vertexShader,
			fragmentShader,
			false);
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlColorPipeline(
			window,
			drawMode,
			vertexShader,
			fragmentShader,
			true);
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
	pipeline->destroyFunction =
		destroyGlColorPipeline;
	pipeline->bindFunction =
		bindGlColorPipelineCommand;
	pipeline->flushFunction =
		flushGlColorPipelineCommand;
	pipeline->handle = handle;
	return pipeline;
}
