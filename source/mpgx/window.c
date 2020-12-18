#include "mpgx/window.h"
#include "mpgx/opengl.h"

#include "ft2build.h"
#include FT_FREETYPE_H

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
static FT_Library ftLibrary = NULL;

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

	if (pipeline->cullFace == FRONT_ONLY_CULL_FACE)
		glCullFace(GL_FRONT);
	else if (pipeline->cullFace == BACK_FRONT_CULL_FACE)
		glCullFace(GL_FRONT_AND_BACK);
	else
		glCullFace(GL_BACK);

	if (pipeline->frontFace == CLOCKWISE_FRONT_FACE)
		glFrontFace(GL_CW);
	else
		glFrontFace(GL_CCW);

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
	assert(indexCount != 0);
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
	assert(indexCount != 0);
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


inline static struct Image* createImage(
	struct Window* window,
	enum ImageType type,
	enum ImageFormat format,
	size_t width,
	size_t height,
	size_t depth,
	const void* pixels,
	bool mipmap)
{
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
struct Image* createImage1D(
	struct Window* window,
	enum ImageFormat format,
	size_t width,
	const void* pixels,
	bool mipmap)
{
	assert(window != NULL);
	assert(width != 0);
	assert(window->recording == false);

	return createImage(
		window,
		IMAGE_1D_TYPE,
		format,
		width,
		1,
		1,
		pixels,
		mipmap);
}
struct Image* createImage2D(
	struct Window* window,
	enum ImageFormat format,
	size_t width,
	size_t height,
	const void* pixels,
	bool mipmap)
{
	assert(window != NULL);
	assert(width != 0);
	assert(height != 0);
	assert(window->recording == false);

	return createImage(
		window,
		IMAGE_1D_TYPE,
		format,
		width,
		height,
		1,
		pixels,
		mipmap);
}
struct Image* createImage3D(
	struct Window* window,
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

	return createImage(
		window,
		IMAGE_1D_TYPE,
		format,
		width,
		height,
		depth,
		pixels,
		mipmap);
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

struct Pipeline* createPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
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
	pipeline->cullFace = cullFace;
	pipeline->frontFace = frontFace;
	pipeline->destroyFunction = destroyFunction;
	pipeline->bindFunction = bindFunction;
	pipeline->setUniformsFunction = setUniformsFunction;
	pipeline->handle = handle;
	return pipeline;
}
void destroyPipeline(
	struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->recording == false);

	if (pipeline == NULL)
		return;

	DestroyPipeline destroyFunction =
		pipeline->destroyFunction;

	destroyFunction(pipeline);
	free(pipeline);
}

struct Window* getPipelineWindow(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->recording == false);
	return pipeline->window;
}
enum DrawMode getPipelineDrawMode(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->recording == false);
	return pipeline->drawMode;
}
enum CullFace getPipelineCullFace(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->recording == false);
	return pipeline->cullFace;
}
enum FrontFace getPipelineFrontFace(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->recording == false);
	return pipeline->frontFace;
}

void bindPipelineCommand(
	struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(pipeline->window->recording == true);

	BindPipelineCommand bindFunction =
		pipeline->bindFunction;
	bindFunction(pipeline);
}

struct GlColorPipeline
{
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
};
struct ColorPipeline
{
	struct Matrix4F mvp;
	struct Vector4F color;
	void* handle;
};

inline static struct GlColorPipeline* createGlColorPipeline(
	struct Window* window,
	const void* vertexShader,
	const void* fragmentShader,
	bool gles)
{
	struct GlColorPipeline* pipeline = malloc(
		sizeof(struct GlColorPipeline));

	if (pipeline == NULL)
		return NULL;

	GLenum stages[2] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
	};
	const char* shaders[2] = {
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

	pipeline->handle = handle;
	pipeline->mvpLocation = mvpLocation;
	pipeline->colorLocation = colorLocation;
	return pipeline;
}
void destroyGlColorPipeline(
	struct Pipeline* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline->handle;
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	glfwMakeContextCurrent(
		pipeline->window->handle);

	glDeleteProgram(
		glColorPipeline->handle);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(glColorPipeline);
	free(colorPipeline);
}
void bindGlColorPipeline(
	struct Pipeline* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline->handle;
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	glUseProgram(glColorPipeline->handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
}
void setGlColorPipelineUniforms(
	struct Pipeline* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline->handle;
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	glUniformMatrix4fv(
		glColorPipeline->mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&colorPipeline->mvp);
	glUniform4fv(
		glColorPipeline->colorLocation,
		1,
		(const GLfloat*)&colorPipeline->color);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector3F),
		NULL);
}
struct Pipeline* createColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace)
{
	assert(window != NULL);
	assert(window->recording == false);

	struct ColorPipeline* colorPipeline =
		malloc(sizeof(struct ColorPipeline));

	if (colorPipeline == NULL)
		return NULL;

	enum GraphicsAPI api =
		window->api;

	void* handle;

	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;

	if (api == OPENGL_GRAPHICS_API)
	{
		handle = createGlColorPipeline(
			window,
			OPENGL_COLOR_VERTEX_SHADER,
			OPENGL_COLOR_FRAGMENT_SHADER,
			false);

		destroyFunction = destroyGlColorPipeline;
		bindFunction = bindGlColorPipeline;
		setUniformsFunction = setGlColorPipelineUniforms;
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlColorPipeline(
			window,
			OPENGL_COLOR_VERTEX_SHADER,
			OPENGL_COLOR_FRAGMENT_SHADER,
			true);

		destroyFunction = destroyGlColorPipeline;
		bindFunction = bindGlColorPipeline;
		setUniformsFunction = setGlColorPipelineUniforms;
	}
	else
	{
		free(colorPipeline);
		return NULL;
	}

	if (handle == NULL)
	{
		free(colorPipeline);
		return NULL;
	}

	colorPipeline->mvp = createIdentityMatrix4F();
	colorPipeline->color = createValueVector4F(1.0f);
	colorPipeline->handle = handle;

	struct Pipeline* pipeline = createPipeline(
		window,
		drawMode,
		cullFace,
		frontFace,
		destroyFunction,
		bindFunction,
		setUniformsFunction,
		colorPipeline);

	if (pipeline == NULL)
	{
		destroyGlColorPipeline(handle);
		free(colorPipeline);
		return NULL;
	}

	return pipeline;
}

void setColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline;
	colorPipeline->mvp = mvp;
}
void setColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline;
	colorPipeline->color = color;
}
