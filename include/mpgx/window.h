#pragma once
#include "cmmt/vector.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

#define DEFAULT_MIN_MIPMAP_LOD -1000
#define DEFAULT_MAX_MIPMAP_LOD 1000

typedef enum KeyboardKey
{
	UNKNOWN_KEYBOARD_KEY = -1,
	SPACE_KEYBOARD_KEY = 32,
	APOSTROPHE_KEYBOARD_KEY = 39, /* ' */
	COMMA_KEYBOARD_KEY = 44, /* , */
	MINUS_KEYBOARD_KEY = 45, /* - */
	PERIOD_KEYBOARD_KEY = 46, /* . */
	SLASH_KEYBOARD_KEY = 47, /* / */
	N0_KEYBOARD_KEY = 48,
	N1_KEYBOARD_KEY = 49,
	N2_KEYBOARD_KEY = 50,
	N3_KEYBOARD_KEY = 51,
	N4_KEYBOARD_KEY = 52,
	N5_KEYBOARD_KEY = 53,
	N6_KEYBOARD_KEY = 54,
	N7_KEYBOARD_KEY = 55,
	N8_KEYBOARD_KEY = 56,
	N9_KEYBOARD_KEY = 57,
	SEMICOLON_KEYBOARD_KEY = 59, /* ; */
	EQUAL_KEYBOARD_KEY = 61, /* = */
	A_KEYBOARD_KEY = 65,
	B_KEYBOARD_KEY = 66,
	C_KEYBOARD_KEY = 67,
	D_KEYBOARD_KEY = 68,
	E_KEYBOARD_KEY = 69,
	F_KEYBOARD_KEY = 70,
	G_KEYBOARD_KEY = 71,
	H_KEYBOARD_KEY = 72,
	I_KEYBOARD_KEY = 73,
	J_KEYBOARD_KEY = 74,
	K_KEYBOARD_KEY = 75,
	L_KEYBOARD_KEY = 76,
	M_KEYBOARD_KEY = 77,
	N_KEYBOARD_KEY = 78,
	O_KEYBOARD_KEY = 79,
	P_KEYBOARD_KEY = 80,
	Q_KEYBOARD_KEY = 81,
	R_KEYBOARD_KEY = 82,
	S_KEYBOARD_KEY = 83,
	T_KEYBOARD_KEY = 84,
	U_KEYBOARD_KEY = 85,
	V_KEYBOARD_KEY = 86,
	W_KEYBOARD_KEY = 87,
	X_KEYBOARD_KEY = 88,
	Y_KEYBOARD_KEY = 89,
	Z_KEYBOARD_KEY = 90,
	LEFT_BRACKET_KEYBOARD_KEY = 91, /* [ */
	BACKSLASH_KEYBOARD_KEY = 92, /* \ */
	RIGHT_BRACKET_KEYBOARD_KEY = 93, /* ] */
	GRAVE_ACCENT_KEYBOARD_KEY = 96, /* ` */
	WORLD_1_KEYBOARD_KEY = 161, /* non-US #1 */
	WORLD_2_KEYBOARD_KEY = 162, /* non-US #2 */
	ESCAPE_KEYBOARD_KEY = 256,
	ENTER_KEYBOARD_KEY = 257,
	TAB_KEYBOARD_KEY = 258,
	BACKSPACE_KEYBOARD_KEY = 259,
	INSERT_KEYBOARD_KEY = 260,
	DELETE_KEYBOARD_KEY = 261,
	RIGHT_KEYBOARD_KEY = 262,
	LEFT_KEYBOARD_KEY = 263,
	DOWN_KEYBOARD_KEY = 264,
	UP_KEYBOARD_KEY = 265,
	PAGE_UP_KEYBOARD_KEY = 266,
	PAGE_DOWN_KEYBOARD_KEY = 267,
	HOME_KEYBOARD_KEY = 268,
	END_KEYBOARD_KEY = 269,
	CAPS_LOCK_KEYBOARD_KEY = 280,
	SCROLL_LOCK_KEYBOARD_KEY = 281,
	NUM_LOCK_KEYBOARD_KEY = 282,
	PRINT_SCREEN_KEYBOARD_KEY = 283,
	PAUSE_KEYBOARD_KEY = 284,
	F1_KEYBOARD_KEY = 290,
	F2_KEYBOARD_KEY = 291,
	F3_KEYBOARD_KEY = 292,
	F4_KEYBOARD_KEY = 293,
	F5_KEYBOARD_KEY = 294,
	F6_KEYBOARD_KEY = 295,
	F7_KEYBOARD_KEY = 296,
	F8_KEYBOARD_KEY = 297,
	F9_KEYBOARD_KEY = 298,
	F10_KEYBOARD_KEY = 299,
	F11_KEYBOARD_KEY = 300,
	F12_KEYBOARD_KEY = 301,
	F13_KEYBOARD_KEY = 302,
	F14_KEYBOARD_KEY = 303,
	F15_KEYBOARD_KEY = 304,
	F16_KEYBOARD_KEY = 305,
	F17_KEYBOARD_KEY = 306,
	F18_KEYBOARD_KEY = 307,
	F19_KEYBOARD_KEY = 308,
	F20_KEYBOARD_KEY = 309,
	F21_KEYBOARD_KEY = 310,
	F22_KEYBOARD_KEY = 311,
	F23_KEYBOARD_KEY = 312,
	F24_KEYBOARD_KEY = 313,
	F25_KEYBOARD_KEY = 314,
	KP_0_KEYBOARD_KEY = 320,
	KP_1_KEYBOARD_KEY = 321,
	KP_2_KEYBOARD_KEY = 322,
	KP_3_KEYBOARD_KEY = 323,
	KP_4_KEYBOARD_KEY = 324,
	KP_5_KEYBOARD_KEY = 325,
	KP_6_KEYBOARD_KEY = 326,
	KP_7_KEYBOARD_KEY = 327,
	KP_8_KEYBOARD_KEY = 328,
	KP_9_KEYBOARD_KEY = 329,
	KP_DECIMAL_KEYBOARD_KEY = 330,
	KP_DIVIDE_KEYBOARD_KEY = 331,
	KP_MULTIPLY_KEYBOARD_KEY = 332,
	KP_SUBTRACT_KEYBOARD_KEY = 333,
	KP_ADD_KEYBOARD_KEY = 334,
	KP_ENTER_KEYBOARD_KEY = 335,
	KP_EQUAL_KEYBOARD_KEY = 336,
	LEFT_SHIFT_KEYBOARD_KEY = 340,
	LEFT_CONTROL_KEYBOARD_KEY = 341,
	LEFT_ALT_KEYBOARD_KEY = 342,
	LEFT_SUPER_KEYBOARD_KEY = 343,
	RIGHT_SHIFT_KEYBOARD_KEY = 344,
	RIGHT_CONTROL_KEYBOARD_KEY = 345,
	RIGHT_ALT_KEYBOARD_KEY = 346,
	RIGHT_SUPER_KEYBOARD_KEY = 347,
	MENU_KEYBOARD_KEY = 348,
	LAST_KEYBOARD_KEY = MENU_KEYBOARD_KEY,
} KeyboardKey;

typedef enum MouseButton
{
	N1_MOUSE_BUTTON = 0,
	N2_MOUSE_BUTTON = 1,
	N3_MOUSE_BUTTON = 2,
	N4_MOUSE_BUTTON = 3,
	N5_MOUSE_BUTTON = 4,
	N6_MOUSE_BUTTON = 5,
	N7_MOUSE_BUTTON = 6,
	N8_MOUSE_BUTTON = 7,
	LAST_MOUSE_BUTTON = N8_MOUSE_BUTTON,
	LEFT_MOUSE_BUTTON = N1_MOUSE_BUTTON,
	RIGHT_MOUSE_BUTTON = N2_MOUSE_BUTTON,
	MIDDLE_MOUSE_BUTTON = N3_MOUSE_BUTTON,
} MouseButton;

typedef enum CursorMode
{
	DEFAULT_CURSOR_MODE = 0,
	HIDDEN_CURSOR_MODE = 1,
	LOCKED_CURSOR_MODE = 2,
	CURSOR_MODE_COUNT = 3,
} CursorMode;

typedef enum GraphicsAPI
{
	VULKAN_GRAPHICS_API = 0,
	OPENGL_GRAPHICS_API = 1,
	OPENGL_ES_GRAPHICS_API = 2,
	GRAPHICS_API_COUNT = 3,
} GraphicsAPI;

typedef enum BufferType
{
	VERTEX_BUFFER_TYPE = 0,
	INDEX_BUFFER_TYPE = 1,
	UNIFORM_BUFFER_TYPE = 2,
	BUFFER_TYPE_COUNT = 3,
} BufferType;

typedef enum DrawIndex
{
	UINT16_DRAW_INDEX = 0,
	UINT32_DRAW_INDEX = 1,
	DRAW_INDEX_COUNT = 2,
} DrawIndex;

typedef enum ImageType
{
	IMAGE_2D_TYPE = 0,
	IMAGE_3D_TYPE = 1,
	IMAGE_TYPE_COUNT = 2,
} ImageType;

typedef enum ImageFormat
{
	R8G8B8A8_UNORM_IMAGE_FORMAT = 0,
	R8G8B8A8_SRGB_IMAGE_FORMAT = 1,
	IMAGE_FORMAT_COUNT = 2,
} ImageFormat;

typedef enum ImageFilter
{
	LINEAR_IMAGE_FILTER = 0,
	NEAREST_IMAGE_FILTER = 1,
	IMAGE_FILTER_COUNT = 2,
} ImageFilter;

typedef enum ImageWrap
{
	REPEAT_IMAGE_WRAP = 0,
	MIRRORED_REPEAT_IMAGE_WRAP = 1,
	CLAMP_TO_EDGE_IMAGE_WRAP = 2,
	IMAGE_WRAP_COUNT = 3,
} ImageWrap;

typedef enum ImageCompare
{
	LESS_EQUAL_IMAGE_COMPARE = 0,
	GREATER_EQUAL_IMAGE_COMPARE = 1,
	LESS_IMAGE_COMPARE = 2,
	GREATER_IMAGE_COMPARE = 3,
	EQUAL_IMAGE_COMPARE = 4,
	NOT_EQUAL_IMAGE_COMPARE = 5,
	ALWAYS_IMAGE_COMPARE = 6,
	NEVER_IMAGE_COMPARE = 7,
	IMAGE_COMPARE_COUNT = 8,
} ImageCompare;

typedef enum ShaderType
{
	VERTEX_SHADER_TYPE = 0,
	FRAGMENT_SHADER_TYPE = 1,
	COMPUTE_SHADER_TYPE = 2,
	SHADER_TYPE_COUNT = 3,
} ShaderType;

typedef enum DrawMode
{
	POINTS_DRAW_MODE = 0,
	LINE_STRIP_DRAW_MODE = 1,
	LINE_LOOP_DRAW_MODE = 2,
	LINES_DRAW_MODE = 3,
	TRIANGLE_STRIP_DRAW_MODE = 4,
	TRIANGLE_FAN_DRAW_MODE = 5,
	TRIANGLES_DRAW_MODE = 6,
	DRAW_MODE_COUNT = 7,
} DrawMode;

typedef struct Window* Window;
typedef union Buffer* Buffer;
typedef union Mesh* Mesh;
typedef union Image* Image;
typedef union Sampler* Sampler;
typedef union Framebuffer* Framebuffer;
typedef union Shader* Shader;
typedef struct Pipeline* Pipeline;
typedef struct ImageData* ImageData;

typedef void(*OnWindowUpdate)(
	void* argument);

typedef void(*OnPipelineDestroy)(
	Window window,
	void* pipeline);
typedef void(*OnPipelineBind)(
	Pipeline pipeline);
typedef void(*OnPipelineUniformsSet)(
	Pipeline pipeline);

bool initializeGraphics();
void terminateGraphics();
bool isGraphicsInitialized();

void* getFtLibrary();

Window createWindow(
	uint8_t api,
	Vec2U size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool isVisible,
	size_t bufferCapacity,
	size_t meshCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t framebufferCapacity,
	size_t shaderCapacity,
	size_t pipelineCapacity);
Window createAnyWindow(
	Vec2U size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool isVisible,
	size_t bufferCapacity,
	size_t meshCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t framebufferCapacity,
	size_t shaderCapacity,
	size_t pipelineCapacity);
void destroyWindow(Window window);

bool isWindowEmpty(Window window);
uint8_t getWindowGraphicsAPI(Window window);
OnWindowUpdate getWindowOnUpdate(Window window);
void* getWindowUpdateArgument(Window window);
uint32_t getWindowMaxImageSize(Window window);
double getWindowUpdateTime(Window window);
double getWindowDeltaTime(Window window);
Vec2F getWindowContentScale(Window window);
Vec2U getWindowFramebufferSize(Window window);
const char* getWindowClipboard(Window window);

bool getWindowKeyboardKey(
	Window window,
	int key);
bool getWindowMouseButton(
	Window window,
	int button);

Vec2U getWindowSize(
	Window window);
void setWindowSize(
	Window window,
	Vec2U size);

Vec2I getWindowPosition(
	Window window);
void setWindowPosition(
	Window window,
	Vec2I position);

Vec2F getWindowCursorPosition(
	Window window);
void setWindowCursorPosition(
	Window window,
	Vec2F position);

uint8_t getWindowCursorMode(
	Window window);
void setWindowCursorMode(
	Window window,
	uint8_t cursorMode);

bool isWindowFocused(Window window);
bool isWindowIconified(Window window);
bool isWindowMaximized(Window window);
bool isWindowVisible(Window window);
bool isWindowHovered(Window window);

void iconifyWindow(Window window);
void maximizeWindow(Window window);
void restoreWindow(Window window);
void showWindow(Window window);
void hideWindow(Window window);
void focusWindow(Window window);
void requestWindowAttention(Window window);

void makeWindowContextCurrent(Window window);
void updateWindow(Window window);

void beginWindowRender(Window window);
void endWindowRender(Window window);

Buffer createBuffer(
	Window window,
	uint8_t type,
	const void* data,
	size_t size,
	bool isConstant);
void destroyBuffer(Buffer buffer);

Window getBufferWindow(Buffer buffer);
uint8_t getBufferType(Buffer buffer);
size_t getBufferSize(Buffer buffer);
bool isBufferConstant(Buffer buffer);
const void* getBufferHandle(Buffer buffer);

void setBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset);

Mesh createMesh(
	Window window,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer);
void destroyMesh(Mesh mesh);

Window getMeshWindow(Mesh mesh);
uint8_t getMeshDrawIndex(Mesh mesh);

size_t getMeshIndexCount(
	Mesh mesh);
void setMeshIndexCount(
	Mesh mesh,
	size_t indexCount);

size_t getMeshIndexOffset(
	Mesh mesh);
void setMeshIndexOffset(
	Mesh mesh,
	size_t indexOffset);

Buffer getMeshVertexBuffer(
	Mesh mesh);
void setMeshVertexBuffer(
	Mesh mesh,
	Buffer vertexBuffer);

Buffer getMeshIndexBuffer(
	Mesh mesh);
void setMeshIndexBuffer(
	Mesh mesh,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer indexBuffer);

void drawMesh(
	Mesh mesh,
	Pipeline pipeline);

ImageData createImageDataFromFile(
	const char* filePath,
	uint8_t channelCount);
void destroyImageData(ImageData imageData);

const uint8_t* getImageDataPixels(ImageData imageData);
Vec2U getImageDataSize(ImageData imageData);
uint8_t getImageDataChannelCount(ImageData imageData);

Image createImage(
	Window window,
	uint8_t type,
	uint8_t format,
	Vec3U size,
	const void** data,
	uint8_t levelCount);
Image createImageFromFile(
	Window window,
	uint8_t format,
	const char* filePath,
	bool generateMipmap);
void destroyImage(Image image);

void setImageData(
	Image image,
	const void* data,
	Vec3U size,
	Vec3U offset);

Window getImageWindow(Image image);
uint8_t getImageType(Image image);
uint8_t getImageFormat(Image image);
Vec3U getImageSize(Image image);
const void* getImageHandle(Image image);
uint8_t getImageLevelCount(Vec3U imageSize);

Sampler createSampler(
	Window window,
	uint8_t minImageFilter,
	uint8_t magImageFilter,
	uint8_t minMipmapFilter,
	bool useMipmapping,
	uint8_t imageWrapX,
	uint8_t imageWrapY,
	uint8_t imageWrapZ,
	uint8_t imageCompare,
	bool useCompare,
	float minMipmapLod,
	float maxMipmapLod);
void destroySampler(Sampler sampler);

Window getSamplerWindow(Sampler sampler);
uint8_t getSamplerMinImageFilter(Sampler sampler);
uint8_t getSamplerMagImageFilter(Sampler sampler);
uint8_t getSamplerMinMipmapFilter(Sampler sampler);
bool isSamplerUseMipmapping(Sampler sampler);
uint8_t getSamplerImageWrapX(Sampler sampler);
uint8_t getSamplerImageWrapY(Sampler sampler);
uint8_t getSamplerImageWrapZ(Sampler sampler);
uint8_t getSamplerImageCompare(Sampler sampler);
bool isSamplerUseCompare(Sampler sampler);
float getSamplerMinMipmapLod(Sampler sampler);
float getSamplerMaxMipmapLod(Sampler sampler);
const void* getSamplerHandle(Sampler sampler);

Shader createShader(
	Window window,
	uint8_t type,
	const void* code,
	size_t size);
Shader createShaderFromFile(
	Window window,
	uint8_t type,
	const char* filePath);
void destroyShader(Shader shader);

Window getShaderWindow(Shader shader);
uint8_t getShaderType(Shader shader);
const void* getShaderHandle(Shader shader);

Pipeline createPipeline(
	Window window,
	const char* name,
	uint8_t drawMode,
	OnPipelineDestroy onDestroy,
	OnPipelineBind onBind,
	OnPipelineUniformsSet onUniformsSet,
	void* handle);
void destroyPipeline(Pipeline pipeline);

Window getPipelineWindow(Pipeline pipeline);
const char* getPipelineName(Pipeline pipeline);
OnPipelineDestroy getPipelineOnDestroy(Pipeline pipeline);
OnPipelineBind getPipelineOnBind(Pipeline pipeline);
OnPipelineUniformsSet getPipelineOnUniformsSet(Pipeline pipeline);
void* getPipelineHandle(Pipeline pipeline);

uint8_t getPipelineDrawMode(
	Pipeline pipeline);
void setPipelineDrawMode(
	Pipeline pipeline,
	uint8_t drawMode);

void bindPipeline(Pipeline pipeline);
