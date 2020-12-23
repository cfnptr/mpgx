#pragma once
#include "mpgx/vector.h"
#include "mpgx/matrix.h"

#include <stdlib.h>

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

enum ImageFilter
{
	LINEAR_IMAGE_FILTER,
	NEAREST_IMAGE_FILTER,
};

enum ImageWrap
{
	REPEAT_IMAGE_WRAP,
	MIRRORED_REPEAT_IMAGE_WRAP,
	CLAMP_TO_EDGE_IMAGE_WRAP,
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

enum CullFace
{
	BACK_ONLY_CULL_FACE,
	FRONT_ONLY_CULL_FACE,
	BACK_FRONT_CULL_FACE,
};
enum FrontFace
{
	CLOCKWISE_FRONT_FACE,
	COUNTERCLOCKWISE_FRONT_FACE,
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
	enum CullFace cullFace;
	enum FrontFace frontFace;
	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;
	void* handle;
};

bool initializeGraphics();
void terminateGraphics();

void* getFtLibrary();

struct Window* createWindow(
	enum GraphicsAPI api,
	size_t width,
	size_t height,
	const char* title);
void destroyWindow(
	struct Window* window);

enum GraphicsAPI getWindowGraphicsAPI(
	const struct Window* window);
double getWindowUpdateTime(
	const struct Window* window);
double getWindowDeltaTime(
	const struct Window* window);
void getWindowSize(
	const struct Window* window,
	size_t* width,
	size_t* height);
void getWindowPosition(
	const struct Window* window,
	size_t* x,
	size_t* y);
void getWindowFramebufferSize(
	const struct Window* window,
	size_t* width,
	size_t* height);

void makeWindowContextCurrent(
	struct Window* window);
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
	size_t indexCount);

struct Buffer* getMeshVertexBuffer(
	const struct Mesh* mesh);
void setMeshVertexBuffer(
	struct Mesh* mesh,
	struct Buffer* vertexBuffer);

struct Buffer* getMeshIndexBuffer(
	const struct Mesh* mesh);
void setMeshIndexBuffer(
	struct Mesh* mesh,
	enum DrawIndex drawIndex,
	size_t indexCount,
	struct Buffer* indexBuffer);

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

void setImageData(
	struct Image* image,
	const void* data,
	size_t width,
	size_t height,
	size_t depth,
	size_t widthOffset,
	size_t heightOffset,
	size_t depthOffset,
	size_t mipmapLevel);
void generateMipmap(
	struct Image* image);

struct Window* getImageWindow(
	const struct Image* image);
enum ImageType getImageType(
	const struct Image* image);
enum ImageFormat getImageFormat(
	const struct Image* image);
size_t getImageWidth(
	const struct Image* image);
size_t getImageHeight(
	const struct Image* image);
size_t getImageDepth(
	const struct Image* image);
bool getImageMipmap(
	const struct Image* image);
const void* getImageHandle(
	const struct Image* image);

struct Pipeline* createPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
	DestroyPipeline destroyFunction,
	BindPipelineCommand bindFunction,
	SetUniformsCommand setUniformsFunction,
	void* handle);
void destroyPipeline(
	struct Pipeline* pipeline);

struct Window* getPipelineWindow(
	const struct Pipeline* pipeline);
enum DrawMode getPipelineDrawMode(
	const struct Pipeline* pipeline);
enum CullFace getPipelineCullFace(
	const struct Pipeline* pipeline);
enum FrontFace getPipelineFrontFace(
	const struct Pipeline* pipeline);

void bindPipelineCommand(
	struct Pipeline* pipeline);

// TODO: create shader objects for optimization
// Shaders could be potentially shared between same pipelines
// Do not allow shader destruction before pipeline destruction
