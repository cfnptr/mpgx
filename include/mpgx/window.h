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

enum DrawIndex
{
	UINT16_DRAW_MODE,
	UINT32_DRAW_MODE,
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

enum ShaderStage
{
	VERTEX_SHADER_STAGE,
	FRAGMENT_SHADER_STAGE,
	COMPUTE_SHADER_STAGE,
	// TODO: other shader stages
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

enum UniformType
{
	VECTOR_4F_UNIFORM_TYPE,
	MATRIX_4F_UNIFORM_TYPE,
};

struct UniformData
{
	const char* name;
	enum UniformType type;
};

struct Window;
struct Buffer;
struct Mesh;
struct Image;
//struct Framebuffer;
struct Shader;
struct Pipeline;
//struct Camera;

typedef void(*WindowRender)(void*);

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
	const struct Window* window);
double getWindowDeltaTime(
	const struct Window* window);

void startWindowUpdate(
	struct Window* window,
	WindowRender renderFunction,
	void* functionArgument);

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
	struct Buffer* buffer);

void drawMeshCommand(
	struct Mesh* mesh);

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

struct Shader* createShader(
	struct Window* window,
	enum ShaderStage stage,
	const void* program,
	size_t size);
void destroyShader(
	struct Shader* shader);

struct Window* getShaderWindow(
	const struct Shader* shader);
enum ShaderStage getShaderStage(
	const struct Shader* shader);

struct Pipeline* createPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	const struct Shader** shaders,
	size_t shaderCount,
	const struct UniformData* uniforms,
	size_t uniformCount);
void destroyPipeline(
	struct Pipeline* pipeline);

void setUniformCommand(
	struct Pipeline* pipeline,
	size_t index,
	const void* data);

struct Shader* createColorVertexShader(
	struct Window* window);
struct Shader* createColorFragmentShader(
	struct Window* window);
struct Pipeline* createColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	const struct Shader* vertexShader,
	const struct Shader* fragmentShader);

