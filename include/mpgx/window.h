#pragma once
#include <stdlib.h>
#include <stdbool.h>

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

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
	// TODO:
	TODO
};

enum ShaderStage
{
	VERTEX_SHADER_STAGE,
	FRAGMENT_SHADER_STAGE,
	COMPUTE_SHADER_STAGE,
	// TODO: other shader stages
};

struct Window;
struct Buffer;
struct Image;
struct Shader;
struct Mesh;
//struct Pipeline;
//struct Framebuffer;

bool initializeGraphics();
void terminateGraphics();
bool getGraphicsInitialized();

struct Window* createWindow(
	enum GraphicsAPI api,
	size_t width,
	size_t height,
	const char* title);
void destroyWindow(
	struct Window* window);

double getWindowUpdateTime(
	struct Window* window);
double getWindowDeltaTime(
	struct Window* window);

void startWindowUpdate(
	struct Window* window);

struct Buffer* createBuffer(
	struct Window* window,
	enum BufferType type,
	const void* data,
	size_t size,
	bool constant);
void destroyBuffer(
	struct Buffer* buffer);

enum GraphicsAPI getBufferAPI(
	const struct Buffer* buffer);
enum BufferType getBufferType(
	const struct Buffer* buffer);
size_t getBufferSize(
	const struct Buffer* buffer);
bool getBufferConstant(
	const struct Buffer* buffer);

bool setBufferData(
	struct Buffer* buffer,
	void* data,
	size_t size,
	size_t offset);

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

// TODO: get image properties

struct Shader* createShader(
	struct Window* window,
	enum ShaderStage stage,
	const void* program,
	size_t size);
void destroyShader(
	struct Shader* shader);

enum ShaderStage getShaderStage(
	struct Shader* shader);

// TODO: get shader program

struct Mesh* createMesh(
	struct Window* window,
	size_t indexCount,
	const void* vertexData,
	size_t vertexSize,
	const void* indexData,
	size_t indexSize,
	bool constant);
void destroyMesh(
	struct Mesh* mesh);
