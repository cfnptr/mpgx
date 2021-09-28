#pragma once
#include "cmmt/vector.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

#define DEFAULT_MIN_MIPMAP_LOD -1000
#define DEFAULT_MAX_MIPMAP_LOD 1000
#define DEFAULT_MIPMAP_LOD_BIAS 0
#define DEFAULT_LINE_WIDTH 1
#define DEFAULT_MIN_DEPTH_RANGE 0
#define DEFAULT_MAX_DEPTH_RANGE 1

// TODO: fix a new framebuffer sRGB difference

// TODO: use glPolygonOffset to improve shadow mapping
// https://docs.microsoft.com/en-gb/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps?redirectedfrom=MSDN

static const Vec2U defaultWindowSize = {
	DEFAULT_WINDOW_WIDTH,
	DEFAULT_WINDOW_HEIGHT,
};
static const Vec2F defaultDepthRange = {
	DEFAULT_MIN_DEPTH_RANGE,
	DEFAULT_MAX_DEPTH_RANGE,
};

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

typedef enum ImageType
{
	IMAGE_1D_TYPE = 0,
	IMAGE_2D_TYPE = 1,
	IMAGE_3D_TYPE = 2,
	IMAGE_TYPE_COUNT = 3,
} ImageType;

typedef enum ImageFormat
{
	R8G8B8A8_UNORM_IMAGE_FORMAT = 0,
	R8G8B8A8_SRGB_IMAGE_FORMAT = 1,
	D16_UNORM_IMAGE_FORMAT = 2,
	D32_SFLOAT_IMAGE_FORMAT = 3,
	D24_UNORM_S8_UINT_IMAGE_FORMAT = 4,
	D32_SFLOAT_S8_UINT_IMAGE_FORMAT = 5,
	IMAGE_FORMAT_COUNT = 6,
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
	CLAMP_TO_BORDER_IMAGE_WRAP = 3,
	MIRROR_CLAMP_TO_EDGE_IMAGE_WRAP = 4,
	IMAGE_WRAP_COUNT = 5,
} ImageWrap;

typedef enum ShaderType
{
	VERTEX_SHADER_TYPE = 0,
	FRAGMENT_SHADER_TYPE = 1,
	COMPUTE_SHADER_TYPE = 2,
	TESSELLATION_CONTROL_SHADER_TYPE = 3,
	TESSELLATION_EVALUATION_SHADER_TYPE = 4,
	GEOMETRY_SHADER_TYPE = 5,
	SHADER_TYPE_COUNT = 6,
} ShaderType;

typedef enum DrawMode
{
	POINT_LIST_DRAW_MODE = 0,
	LINE_STRIP_DRAW_MODE = 1,
	LINE_LOOP_DRAW_MODE = 2,
	LINE_LIST_DRAW_MODE = 3,
	TRIANGLE_STRIP_DRAW_MODE = 4,
	TRIANGLE_FAN_DRAW_MODE = 5,
	TRIANGLE_LIST_DRAW_MODE = 6,
	LINE_LIST_WITH_ADJACENCY_DRAW_MODE = 7,
	LINE_STRIP_WITH_ADJACENCY_DRAW_MODE = 8,
	TRIANGLE_LIST_WITH_ADJACENCY_DRAW_MODE = 9,
	TRIANGLE_STRIP_WITH_ADJACENCY_DRAW_MODE = 10,
	PATCH_LIST_DRAW_MODE = 11,
	DRAW_MODE_COUNT = 12,
} DrawMode;

typedef enum PolygonMode
{
	POINT_POLYGON_MODE = 0,
	LINE_POLYGON_MODE = 1,
	FILL_POLYGON_MODE = 2,
	POLYGON_MODE_COUNT = 3,
} PolygonMode;

typedef enum CullMode
{
	FRONT_CULL_MODE = 0,
	BACK_CULL_MODE = 1,
	FRONT_AND_BACK_CULL_MODE = 2,
	CULL_MODE_COUNT = 3,
} CullMode;

typedef enum CompareOperator
{
	LESS_OR_EQUAL_COMPARE_OPERATOR = 0,
	GREATER_OR_EQUAL_COMPARE_OPERATOR = 1,
	LESS_COMPARE_OPERATOR = 2,
	GREATER_COMPARE_OPERATOR = 3,
	EQUAL_COMPARE_OPERATOR = 4,
	NOT_EQUAL_COMPARE_OPERATOR = 5,
	ALWAYS_COMPARE_OPERATOR = 6,
	NEVER_COMPARE_OPERATOR = 7,
	COMPARE_OPERATOR_COUNT = 8,
} CompareOperator;

typedef enum ColorComponent
{
	NONE_COLOR_COMPONENT = 0b0000,
	RED_COLOR_COMPONENT = 0b0001,
	GREEN_COLOR_COMPONENT = 0b0010,
	BLUE_COLOR_COMPONENT = 0b0100,
	ALPHA_COLOR_COMPONENT = 0b1000,
	ALL_COLOR_COMPONENT = 0b1111,
} ColorComponent;

typedef enum BlendFactor
{
	ZERO_BLEND_FACTOR = 0,
	ONE_BLEND_FACTOR = 1,
	SRC_COLOR_BLEND_FACTOR = 2,
	ONE_MINUS_SRC_COLOR_BLEND_FACTOR = 3,
	DST_COLOR_BLEND_FACTOR = 4,
	ONE_MINUS_DST_COLOR_BLEND_FACTOR = 5,
	SRC_ALPHA_BLEND_FACTOR = 6,
	ONE_MINUS_SRC_ALPHA_BLEND_FACTOR = 7,
	DST_ALPHA_BLEND_FACTOR = 8,
	ONE_MINUS_DST_ALPHA_BLEND_FACTOR = 9,
	CONSTANT_COLOR_BLEND_FACTOR = 10,
	ONE_MINUS_CONSTANT_COLOR_BLEND_FACTOR = 11,
	CONSTANT_ALPHA_BLEND_FACTOR = 12,
	ONE_MINUS_CONSTANT_ALPHA_BLEND_FACTOR = 13,
	SRC_ALPHA_SATURATE_BLEND_FACTOR = 14,
	SRC1_COLOR_BLEND_FACTOR = 15,
	ONE_MINUS_SRC1_COLOR_BLEND_FACTOR = 16,
	SRC1_ALPHA_BLEND_FACTOR = 17,
	ONE_MINUS_SRC1_ALPHA_BLEND_FACTOR = 18,
	BLEND_FACTOR_COUNT = 19,
} BlendFactor;

typedef enum BlendOperator
{
	ADD_BLEND_OPERATOR = 0,
	SUBTRACT_BLEND_OPERATOR = 1,
	REVERSE_SUBTRACT_BLEND_OPERATOR = 2,
	MIN_BLEND_OPERATOR = 3,
	MAX_BLEND_OPERATOR = 4,
	BLEND_OPERATOR_COUNT = 5,
} BlendOperator;

typedef enum DrawIndex
{
	UINT16_DRAW_INDEX = 0,
	UINT32_DRAW_INDEX = 1,
	DRAW_INDEX_COUNT = 2,
} DrawIndex;

typedef struct PipelineState
{
	uint8_t drawMode;
	uint8_t polygonMode;
	uint8_t cullMode;
	uint8_t depthCompareOperator;
	uint8_t colorComponentWriteMask;
	uint8_t srcColorBlendFactor;
	uint8_t dstColorBlendFactor;
	uint8_t srcAlphaBlendFactor;
	uint8_t dstAlphaBlendFactor;
	uint8_t colorBlendOperator;
	uint8_t alphaBlendOperator;
	bool cullFace;
	bool clockwiseFrontFace;
	bool testDepth;
	bool writeDepth;
	bool clampDepth;
	bool enableBlend;
	bool restartPrimitive;
	bool discardRasterizer;
	float lineWidth;
	Vec4I viewport;
	Vec2F depthRange;
	Vec4I scissor;
	Vec4F blendColor;
} PipelineState;

typedef struct Window* Window;
typedef union Buffer* Buffer;
typedef union Mesh* Mesh;
typedef union Image* Image;
typedef union Sampler* Sampler;
typedef union Framebuffer* Framebuffer;
typedef union Shader* Shader;
typedef union Pipeline* Pipeline;
typedef struct ImageData* ImageData;

typedef void(*OnWindowUpdate)(void* argument);
typedef void(*OnPipelineHandleDestroy)(Window window, void* handle);
typedef void(*OnPipelineHandleBind)(Pipeline pipeline);
typedef void(*OnPipelineUniformsSet)(Pipeline pipeline);
typedef void(*OnPipelineHandleResize)(Pipeline pipeline, void* createInfo);

bool initializeGraphics(
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch);
void terminateGraphics();
bool isGraphicsInitialized();

void* getFtLibrary();

Window createWindow(
	uint8_t api,
	bool useStencilBuffer,
	Vec2U size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool isVisible,
	size_t bufferCapacity,
	size_t imageCapacity,
	size_t samplerCapacity,
	size_t framebufferCapacity,
	size_t shaderCapacity,
	size_t pipelineCapacity,
	size_t meshCapacity);
Window createAnyWindow(
	bool useStencilBuffer,
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
bool isWindowUseStencilBuffer(Window window);
OnWindowUpdate getWindowOnUpdate(Window window);
void* getWindowUpdateArgument(Window window);
double getWindowUpdateTime(Window window);
double getWindowDeltaTime(Window window);
Vec2F getWindowContentScale(Window window);
Vec2U getWindowFramebufferSize(Window window);
const char* getWindowClipboard(Window window);
const char* getWindowGpuName(Window window);

void* getVkWindow(Window window);
bool isVkGpuIntegrated(Window window);

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

void beginWindowRender(
	Window window,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil);
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

void setBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset);

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
	uint8_t compareOperation,
	bool useCompare,
	float minMipmapLod,
	float maxMipmapLod,
	float mipmapLodBias);
void destroySampler(Sampler sampler);

Window getSamplerWindow(Sampler sampler);
uint8_t getSamplerMinImageFilter(Sampler sampler);
uint8_t getSamplerMagImageFilter(Sampler sampler);
uint8_t getSamplerMinMipmapFilter(Sampler sampler);
bool isSamplerUseMipmapping(Sampler sampler);
uint8_t getSamplerImageWrapX(Sampler sampler);
uint8_t getSamplerImageWrapY(Sampler sampler);
uint8_t getSamplerImageWrapZ(Sampler sampler);
uint8_t getSamplerCompareOperation(Sampler sampler);
bool isSamplerUseCompare(Sampler sampler);
float getSamplerMinMipmapLod(Sampler sampler);
float getSamplerMaxMipmapLod(Sampler sampler);
float getSamplerMipmapLodBias(Sampler sampler);

Framebuffer createFramebuffer(
	Window window,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment);
void destroyFramebuffer(Framebuffer framebuffer);

Image* getFramebufferColorAttachments(
	Framebuffer framebuffer);
size_t getFramebufferColorAttachmentCount(
	Framebuffer framebuffer);
Image getFramebufferDepthStencilAttachment(
	Framebuffer framebuffer);

void beginFramebufferRender(Framebuffer framebuffer);
void endFramebufferRender(Window window);

void clearFramebuffer(
	Window window,
	bool clearColorBuffer,
	bool clearDepthBuffer,
	bool clearStencilBuffer,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil);

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

Pipeline createPipeline(
	Window window,
	const char* name,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	OnPipelineHandleDestroy onHandleDestroy,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineHandleResize onHandleResize,
	void* handle,
	void* createInfo);
void destroyPipeline(
	Pipeline pipeline,
	bool destroyShaders);

Window getPipelineWindow(Pipeline pipeline);
const char* getPipelineName(Pipeline pipeline);
Shader* getPipelineShaders(Pipeline pipeline);
uint8_t getPipelineShaderCount(Pipeline pipeline);
const PipelineState* getPipelineState(Pipeline pipeline);
OnPipelineHandleDestroy getPipelineOnHandleDestroy(Pipeline pipeline);
OnPipelineHandleBind getPipelineOnHandleBind(Pipeline pipeline);
OnPipelineUniformsSet getPipelineOnUniformsSet(Pipeline pipeline);
void* getPipelineHandle(Pipeline pipeline);

void bindPipeline(Pipeline pipeline);

Mesh createMesh(
	Window window,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer);
void destroyMesh(
	Mesh mesh,
	bool destroyBuffers);

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

size_t drawMesh(
	Mesh mesh,
	Pipeline pipeline);
