#include "mpgx/window.h"
#include "mpgx/vector.h"
#include "mpgx/matrix.h"
#include "mpgx/opengl.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

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
	struct Mesh*,
	struct Pipeline*);

struct GlMesh
{
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

void beginGlCommandRecord(
	struct Window* window)
{
	// TODO: move to the framebuffer
	glClear(
		GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);
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
	assert(window->recording == false);
	return window->updateTime;
}
double getWindowDeltaTime(
	const struct Window* window)
{
	assert(window != NULL);
	assert(window->recording == false);
	return window->deltaTime;
}

void startWindowUpdate(
	struct Window* window,
	WindowRender renderFunction,
	void* functionArgument)
{
	assert(window != NULL);
	assert(renderFunction != NULL);
	assert(window->recording == false);

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
	assert(window->recording == false);

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
		free(buffer);
		return NULL;
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
	assert(buffer->window->recording == false);

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
	assert(buffer->window->recording == false);
	return buffer->window;
}
enum BufferType getBufferType(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	assert(buffer->window->recording == false);
	return buffer->type;
}
size_t getBufferSize(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	assert(buffer->window->recording == false);
	return buffer->size;
}
bool getBufferConstant(
	const struct Buffer* buffer)
{
	assert(buffer != NULL);
	assert(buffer->window->recording == false);
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
	enum DrawIndex _drawIndex)
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

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

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

	SetUniformsCommand setUniformsFunction =
		pipeline->setUniformsFunction;
	setUniformsFunction(pipeline);

	GLenum glDrawMode;

	// TODO: Optimize this
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

	enum GraphicsAPI api =
		window->api;

	void* handle;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlMesh(
			window,
			drawIndex);

		mesh->destroyFunction = destroyGlMesh;
		mesh->drawFunction = drawGlMeshCommand;
	}
	else
	{
		free(mesh);
		return NULL;
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
	assert(mesh->window->recording == false);

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
	assert(mesh->window->recording == false);
	return mesh->window;
}
enum DrawIndex getMeshDrawIndex(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	assert(mesh->window->recording == false);
	return mesh->drawIndex;
}

size_t getMeshIndexCount(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	assert(mesh->window->recording == false);
	return mesh->indexCount;
}
void setMeshIndexCount(
	struct Mesh* mesh,
	size_t count)
{
	assert(mesh != NULL);
	assert(mesh->window->recording == false);

	assert(mesh->drawIndex == UINT32_DRAW_INDEX ?
		count * sizeof(uint32_t) <= mesh->indexBuffer->size :
		count * sizeof(uint16_t) <= mesh->indexBuffer->size);

	mesh->indexCount = count;
}

struct Buffer* getMeshVertexBuffer(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	assert(mesh->window->recording == false);
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
	assert(mesh->window->recording == false);
	mesh->vertexBuffer = buffer;
}

struct Buffer* getMeshIndexBuffer(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	assert(mesh->window->recording == false);
	return mesh->indexBuffer;
}
void setMeshIndexBuffer(
	struct Mesh* mesh,
	enum DrawIndex drawIndex,
	size_t indexCount,
	struct Buffer* buffer)
{
	assert(mesh != NULL);
	assert(buffer != NULL);
	assert(mesh->window == buffer->window);
	assert(buffer->type == INDEX_BUFFER_TYPE);
	assert(mesh->window->recording == false);

	assert(mesh->drawIndex == UINT32_DRAW_INDEX ?
		indexCount * sizeof(uint32_t) <= mesh->indexBuffer->size :
		indexCount * sizeof(uint16_t) <= mesh->indexBuffer->size);

	mesh->drawIndex = drawIndex;
	mesh->indexCount = indexCount;
	mesh->indexBuffer = buffer;
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
	enum DrawIndex drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer)
{
	assert(mesh != NULL);
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);
	assert(mesh->window == vertexBuffer->window);
	assert(mesh->window == indexBuffer->window);
	assert(vertexBuffer->type == VERTEX_BUFFER_TYPE);
	assert(indexBuffer->type == INDEX_BUFFER_TYPE);
	assert(mesh->window->recording == false);

	assert(mesh->drawIndex == UINT32_DRAW_INDEX ?
		indexCount * sizeof(uint32_t) <= mesh->indexBuffer->size :
		indexCount * sizeof(uint16_t) <= mesh->indexBuffer->size);

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
	assert(pipeline->setUniformsFunction != NULL);
	assert(mesh->window->recording == true);

	DrawMeshCommand drawFunction =
		mesh->drawFunction;

	drawFunction(
		mesh,
		pipeline);
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

	if (mipmap == true)
		glGenerateMipmap(type);

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
	assert(window->recording == false);

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
		free(image);
		return NULL;
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
	assert(image->window->recording == false);

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
	assert(image->window->recording == false);
	return image->window;
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

	GLuint handle = createGlPipeline(
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

	pipeline->mvp = createIdentityMatrix4F();
	pipeline->color = createValueVector4F(1.0f);
	pipeline->handle = handle;
	pipeline->mvpLocation = mvpLocation;
	pipeline->colorLocation = colorLocation;
	return pipeline;
}
void destroyGlColorPipeline(
	struct Pipeline* pipeline)
{
	struct GlColorPipeline* glPipeline =
		(struct GlColorPipeline*)pipeline->handle;

	glfwMakeContextCurrent(
		pipeline->window->handle);

	glDeleteProgram(
		glPipeline->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(glPipeline);
}
void bindGlColorPipeline(
	struct Pipeline* pipeline)
{
	struct GlColorPipeline* glPipeline =
		(struct GlColorPipeline*)pipeline->handle;

	glUseProgram(
		glPipeline->handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	// TODO move to constructor
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
}
void setGlColorPipelineUniforms(
	struct Pipeline* pipeline)
{
	struct GlColorPipeline* glPipeline =
		(struct GlColorPipeline*)pipeline->handle;

	glUniformMatrix4fv(
		glPipeline->mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&glPipeline->mvp);
	glUniform4fv(
		glPipeline->colorLocation,
		1,
		(const GLfloat*)&glPipeline->color);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector3F),
		NULL);
}

void destroyPipeline(
	struct Pipeline* pipeline)
{
	assert(pipeline->destroyFunction != NULL);
	assert(pipeline->window->recording == false);

	if (pipeline == NULL)
		return;

	DestroyPipeline destroyFunction =
		pipeline->destroyFunction;

	destroyFunction(pipeline);
	free(pipeline);
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

struct Pipeline* createColorPipeline(
	struct Window* window,
	enum DrawMode drawMode)
{
	assert(window != NULL);
	assert(window->recording == false);

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
			OPENGL_COLOR_VERTEX_SHADER,
			OPENGL_COLOR_FRAGMENT_SHADER,
			false);

		pipeline->destroyFunction =
			destroyGlColorPipeline;
		pipeline->bindFunction =
			bindGlColorPipeline;
		pipeline->setUniformsFunction =
			setGlColorPipelineUniforms;
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlColorPipeline(
			window,
			drawMode,
			OPENGL_COLOR_VERTEX_SHADER,
			OPENGL_COLOR_FRAGMENT_SHADER,
			true);

		pipeline->destroyFunction =
			destroyGlColorPipeline;
		pipeline->bindFunction =
			bindGlColorPipeline;
		pipeline->setUniformsFunction =
			setGlColorPipelineUniforms;
	}
	else
	{
		free(pipeline);
		return NULL;
	}

	if (handle == NULL)
	{
		free(pipeline);
		return NULL;
	}

	pipeline->window = window;
	pipeline->drawMode = drawMode;
	pipeline->handle = handle;
	return pipeline;
}
