#pragma once
#include "mpgx/vector.h"
#include "mpgx/matrix.h"

#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

// TODO:
// Add other enumerations

enum GraphicsAPI
{
	VULKAN_GRAPHICS_API = 0,
	OPENGL_GRAPHICS_API = 1,
	OPENGL_ES_GRAPHICS_API = 2,
};

enum BufferType
{
	VERTEX_BUFFER_TYPE = 0,
	INDEX_BUFFER_TYPE = 1,
	UNIFORM_BUFFER_TYPE = 2,
};

enum DrawIndex
{
	UINT8_DRAW_INDEX = 0,
	UINT32_DRAW_INDEX = 1,
	UINT16_DRAW_INDEX = 2,
};

enum ImageType
{
	IMAGE_2D_TYPE = 0,
	IMAGE_3D_TYPE = 1,
};

enum ImageFormat
{
	R8G8B8A8_UNORM_IMAGE_FORMAT = 0,
	R8G8B8A8_SRGB_IMAGE_FORMAT = 1,
};

enum ImageFilter
{
	LINEAR_IMAGE_FILTER = 0,
	NEAREST_IMAGE_FILTER = 1,
};

enum ImageWrap
{
	REPEAT_IMAGE_WRAP = 0,
	MIRRORED_REPEAT_IMAGE_WRAP = 1,
	CLAMP_TO_EDGE_IMAGE_WRAP = 2,
};

enum ShaderType
{
	VERTEX_SHADER_TYPE = 0,
	FRAGMENT_SHADER_TYPE = 1,
	COMPUTE_SHADER_TYPE = 2,
};

enum DrawMode
{
	POINTS_DRAW_MODE = 0,
	LINE_STRIP_DRAW_MODE = 1,
	LINE_LOOP_DRAW_MODE = 2,
	LINES_DRAW_MODE = 3,
	TRIANGLE_STRIP_DRAW_MODE = 4,
	TRIANGLE_FAN_DRAW_MODE = 5,
	TRIANGLES_DRAW_MODE = 6,
};

enum CullFace
{
	BACK_ONLY_CULL_FACE = 0,
	FRONT_ONLY_CULL_FACE = 1,
	BACK_FRONT_CULL_FACE = 2,
};
enum FrontFace
{
	CLOCKWISE_FRONT_FACE = 0,
	COUNTERCLOCKWISE_FRONT_FACE = 1,
};

struct Window;
struct Buffer;
struct Mesh;
struct Image;
struct Framebuffer;
struct Shader;
struct Pipeline;
// TODO:
//struct Query
//struct Sampler?

typedef void(*UpdateWindow)(
	void* argument);

typedef void(*DestroyPipeline)(
	struct Window* window,
	void* pipeline);
typedef void(*BindPipelineCommand)(
	struct Pipeline* pipeline);
typedef void(*SetUniformsCommand)(
	struct Pipeline* pipeline);

bool initializeGraphics();
void terminateGraphics();
bool isGraphicsInitialized();

void* getFtLibrary();

struct Window* createWindow(
	uint8_t api,
	size_t width,
	size_t height,
	const char* title,
	UpdateWindow updateFunction,
	void* updateArgument);
struct Window* createAnyWindow(
	size_t width,
	size_t height,
	const char* title,
	UpdateWindow updateFunction,
	void* updateArgument);
void destroyWindow(
	struct Window* window);

uint8_t getWindowGraphicsAPI(
	const struct Window* window);
size_t getWindowMaxImageSize(
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
void getWindowCursorPosition(
	const struct Window* window,
	double* x,
	double* y);

void makeWindowContextCurrent(
	struct Window* window);
void updateWindow(
	struct Window* window);

void beginCommandRecord(
	struct Window* window);
void endCommandRecord(
	struct Window* window);

struct Buffer* createBuffer(
	struct Window* window,
	uint8_t type,
	const void* data,
	size_t size,
	bool constant);
void destroyBuffer(
	struct Buffer* buffer);

struct Window* getBufferWindow(
	const struct Buffer* buffer);
uint8_t getBufferType(
	const struct Buffer* buffer);
size_t getBufferSize(
	const struct Buffer* buffer);
bool isBufferConstant(
	const struct Buffer* buffer);

void setBufferData(
	struct Buffer* buffer,
	const void* data,
	size_t size,
	size_t offset);

struct Mesh* createMesh(
	struct Window* window,
	uint8_t drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer);
void destroyMesh(
	struct Mesh* mesh);

struct Window* getMeshWindow(
	const struct Mesh* mesh);
uint8_t getMeshDrawIndex(
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
	uint8_t drawIndex,
	size_t indexCount,
	struct Buffer* indexBuffer);

void getMeshBuffers(
	const struct Mesh* mesh,
	struct Buffer** vertexBuffer,
	struct Buffer** indexBuffer);
void setMeshBuffers(
	struct Mesh* mesh,
	uint8_t drawIndex,
	size_t indexCount,
	struct Buffer* vertexBuffer,
	struct Buffer* indexBuffer);

void drawMeshCommand(
	struct Mesh* mesh,
	struct Pipeline* pipeline);

struct Image* createImage(
	struct Window* window,
	uint8_t type,
	uint8_t format,
	size_t width,
	size_t height,
	size_t depth,
	const void* pixels,
	bool useMipmap);
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
void generateMipmaps(
	struct Image* image);

struct Window* getImageWindow(
	const struct Image* image);
uint8_t getImageType(
	const struct Image* image);
uint8_t getImageFormat(
	const struct Image* image);
size_t getImageWidth(
	const struct Image* image);
size_t getImageHeight(
	const struct Image* image);
size_t getImageDepth(
	const struct Image* image);
bool isImageUseMipmapping(
	const struct Image* image);
const void* getImageHandle(
	const struct Image* image);

struct Shader* createShader(
	struct Window* window,
	uint8_t type,
	const void* code,
	size_t size);
struct Shader* readShaderFromFile(
	struct Window* window,
	uint8_t type,
	const char* filePath);
void destroyShader(
	struct Shader* shader);

struct Window* getShaderWindow(
	const struct Shader* shader);
uint8_t getShaderType(
	const struct Shader* shader);
const void* getShaderHandle(
	const struct Shader* shader);

struct Pipeline* createPipeline(
	struct Window* window,
	uint8_t drawMode,
	DestroyPipeline destroyFunction,
	BindPipelineCommand bindFunction,
	SetUniformsCommand setUniformsFunction,
	void* handle);
void destroyPipeline(
	struct Pipeline* pipeline);

struct Window* getPipelineWindow(
	const struct Pipeline* pipeline);
uint8_t getPipelineDrawMode(
	const struct Pipeline* pipeline);
void* getPipelineHandle(
	const struct Pipeline* pipeline);

void bindPipelineCommand(
	struct Pipeline* pipeline);
