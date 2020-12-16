#pragma once
#include <stdlib.h>
#include <stdbool.h>

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

typedef void(*WindowRender)(void*);

enum GraphicsAPI
{
	VULKAN_GRAPHICS_API,
	OPENGL_GRAPHICS_API,
	OPENGL_ES_GRAPHICS_API
};

enum BufferType
{
	VERTEX_BUFFER_TYPE,
	INDEX_BUFFER_TYPE,
	UNIFORM_BUFFER_TYPE,
	// TODO: other buffer types
};

enum DrawIndex
{
	UINT16_DRAW_INDEX,
	UINT32_DRAW_INDEX,
};

enum ImageType
{
	IMAGE_1D_TYPE,
	IMAGE_2D_TYPE,
	IMAGE_3D_TYPE,
	// TODO: cubemaps
};

enum ImageFormat
{
	R8G8B8A8_UNORM_IMAGE_FORMAT,
	R8G8B8A8_SRGB_IMAGE_FORMAT,
	// TODO: add other formats
};

enum DrawMode
{
	POINTS_DRAW_MODE,
	LINE_STRIP_DRAW_MODE,
	LINE_LOOP_DRAW_MODE,
	LINES_DRAW_MODE,
	TRIANGLE_STRIP_DRAW_MODE,
	TRIANGLE_FAN_DRAW_MODE,
	TRIANGLES_DRAW_MODE,
	// TODO: other draw modes
};

struct Window;
struct Buffer;
struct Mesh;
struct Image;
//struct Framebuffer;
//struct Camera;

struct Pipeline;

typedef void(*DestroyPipeline)(
	struct Pipeline*);
typedef void(*BindPipelineCommand)(
	struct Pipeline*);
typedef void(*SetUniformsCommand)(
	struct Pipeline*);

struct Pipeline
{
	struct Window* window;
	enum DrawMode drawMode;
	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;
	void* handle;
};

bool initializeGraphics();
void terminateGraphics();

struct Window* createWindow(
	enum GraphicsAPI api,
	size_t width,
	size_t height,
	const char* title);
void destroyWindow(
	struct Window* window);

double getWindowUpdateTime(
	const struct Window* window);
double getWindowDeltaTime(
	const struct Window* window);

void startWindowUpdate(
	struct Window* window,
	WindowRender renderFunction,
	void* functionArgument);

void beginCommandRecord(
	struct Window* window);
void endCommandRecord(
	struct Window* window);

struct Buffer* createBuffer(
	struct Window* window,
	enum BufferType type,
	const void* data,
	size_t size,
	bool constant);
void destroyBuffer(
	struct Buffer* buffer);

struct Window* getBufferWindow(
	const struct Buffer* buffer);
enum BufferType getBufferType(
	const struct Buffer* buffer);
size_t getBufferSize(
	const struct Buffer* buffer);
bool getBufferConstant(
	const struct Buffer* buffer);

void setBufferData(
	struct Buffer* buffer,
	const void* data,
	size_t size,
	size_t offset);

struct Mesh* createMesh(
	struct Window* window,
	enum DrawIndex drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer);
void destroyMesh(
	struct Mesh* mesh);

struct Window* getMeshWindow(
	const struct Mesh* mesh);
enum DrawIndex getMeshDrawIndex(
	const struct Mesh* mesh);

size_t getMeshIndexCount(
	const struct Mesh* mesh);
void setMeshIndexCount(
	struct Mesh* mesh,
	size_t count);

struct Buffer* getMeshVertexBuffer(
	const struct Mesh* mesh);
void setMeshVertexBuffer(
	struct Mesh* mesh,
	struct Buffer* buffer);

struct Buffer* getMeshIndexBuffer(
	const struct Mesh* mesh);
void setMeshIndexBuffer(
	struct Mesh* mesh,
	enum DrawIndex drawIndex,
	size_t indexCount,
	struct Buffer* buffer);

void getMeshBuffers(
	const struct Mesh* mesh,
	struct Buffer** vertexBuffer,
	struct Buffer** indexBuffer);
void setMeshBuffers(
	struct Mesh* mesh,
	enum DrawIndex drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer);

void drawMeshCommand(
	struct Mesh* mesh,
	struct Pipeline* pipeline);

struct Image* createImage(
	struct Window* window,
	enum ImageType type,
	enum ImageFormat format,
	size_t width,
	size_t height,
	size_t depth,
	const void* pixels,
	bool mipmap);
void destroyImage(
	struct Image* image);

struct Window* getImageWindow(
	const struct Image* image);

// TODO: get image properties

void destroyPipeline(
	struct Pipeline* pipeline);
void bindPipelineCommand(
	struct Pipeline* pipeline);

struct Pipeline* createColorPipeline(
	struct Window* window,
	enum DrawMode drawMode);
