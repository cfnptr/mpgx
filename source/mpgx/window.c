#include "mpgx/window.h"
#include "mpgx/opengl.h"

#include "ft2build.h"
#include FT_FREETYPE_H

typedef void(*BeginCommandRecord)(
	struct Window*);
typedef void(*EndCommandRecord)(
	struct Window*);

typedef void(*DestroyBuffer)(
	struct Buffer*);
typedef void(*SetBufferData)(
	struct Buffer*,
	const void*,
	size_t,
	size_t);

typedef void(*DestroyMesh)(
	struct Mesh*);
typedef void(*DrawMeshCommand)(
	struct Mesh*,
	struct Pipeline*);

typedef void(*DestroyImage)(
	struct Image*);
typedef const void*(*GetImageHandle)(
	const struct Image*);

struct Window
{
	enum GraphicsAPI api;
	double updateTime;
	double deltaTime;
	bool recording;
	BeginCommandRecord beginRecordFunction;
	BeginCommandRecord endRecordFunction;
	DestroyBuffer destroyBufferFunction;
	SetBufferData setBufferDataFunction;
	DestroyMesh destroyMeshFunction;
	DrawMeshCommand drawMeshFunction;
	DestroyImage destroyImageFunction;
	GetImageHandle getImageHandleFunction;
	struct GLFWwindow* handle;
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
	enum ImageType type;
	enum ImageFormat format;
	size_t width;
	size_t height;
	size_t depth;
	bool mipmap;
	void* handle;
};

static bool graphicsInitialized = false;
static FT_Library ftLibrary = NULL;

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

	assertOpenGL();

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

	assertOpenGL();

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

	assertOpenGL();
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

	assertOpenGL();

	free(glImage);
}
const void* getGlImageHandle(
	const struct Image* image)
{
	struct GlImage* glImage =
		(struct GlImage*)image->handle;
	return &glImage->handle;
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
		window->destroyBufferFunction =
			destroyGlBuffer;
		window->setBufferDataFunction =
			setGlBufferData;
		window->destroyMeshFunction =
			destroyGlMesh;
		window->drawMeshFunction =
			drawGlMeshCommand;
		window->destroyImageFunction =
			destroyGlImage;
		window->getImageHandleFunction =
			getGlImageHandle;
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
		window->destroyBufferFunction =
			destroyGlBuffer;
		window->setBufferDataFunction =
			setGlBufferData;
		window->destroyMeshFunction =
			destroyGlMesh;
		window->drawMeshFunction =
			drawGlMeshCommand;
		window->destroyImageFunction =
			destroyGlImage;
		window->getImageHandleFunction =
			getGlImageHandle;
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

enum GraphicsAPI getWindowGraphicsAPI(
	const struct Window* window)
{
	assert(window != NULL);
	return window->api;
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

void makeWindowContextCurrent(
	struct Window* window)
{
	assert(window != NULL);

	assert(window->api == OPENGL_GRAPHICS_API ||
		window->api == OPENGL_ES_GRAPHICS_API);

	glfwMakeContextCurrent(
		window->handle);
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
		buffer->window->destroyBufferFunction;

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
	assert(buffer->window->recording == false);

	SetBufferData setDataFunction =
		buffer->window->setBufferDataFunction;

	setDataFunction(
		buffer,
		data,
		size,
		offset);
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
		mesh->window->destroyMeshFunction;

	destroyFunction(mesh);
	free(mesh);
}

struct Window* getMeshWindow(
	const struct Mesh* mesh)
{
	assert(mesh != NULL);
	return mesh->window;
}
enum DrawIndex getMeshDrawIndex(
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
	assert(mesh->window->recording == false);
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
		mesh->window->drawMeshFunction;

	drawFunction(
		mesh,
		pipeline);
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

	image->window = window;
	image->type = type;
	image->format = format;
	image->width = width;
	image->height = height;
	image->depth = depth;
	image->mipmap = mipmap;
	image->handle = handle;
	return image;
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
		IMAGE_2D_TYPE,
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
		IMAGE_3D_TYPE,
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
		image->window->destroyImageFunction;
	destroyFunction(image);

	free(image);
}

struct Window* getImageWindow(
	const struct Image* image)
{
	assert(image != NULL);
	return image->window;
}
enum ImageType getImageType(
	const struct Image* image)
{
	assert(image != NULL);
	return image->type;
}
enum ImageFormat getImageFormat(
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

	GetImageHandle getHandleFunction =
		image->window->getImageHandleFunction;
	return getHandleFunction(image);
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
	return pipeline->window;
}
enum DrawMode getPipelineDrawMode(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	return pipeline->drawMode;
}
enum CullFace getPipelineCullFace(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	return pipeline->cullFace;
}
enum FrontFace getPipelineFrontFace(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
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
