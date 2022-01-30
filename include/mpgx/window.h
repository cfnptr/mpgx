// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "mpgx/defines.h"

#include "cmmt/color.h"
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
#define DEFAULT_MIN_DEPTH 0
#define DEFAULT_MAX_DEPTH 1
#define DEFAULT_DEPTH_BIAS_CONSTANT 0
#define DEFAULT_DEPTH_BIAS_SLOPE 0
#define DEFAULT_BLEND_COLOR 0

// TODO: possibly switch to the ktx texture format
// TODO: add ability to store and load shader cache
// TODO: add window item enumerators and count getters
// TODO: include static vulkan library on MacOS

static const Vec2I defaultWindowSize = {
	DEFAULT_WINDOW_WIDTH,
	DEFAULT_WINDOW_HEIGHT,
};
static const Vec2F defaultMipmapLodRange = {
	DEFAULT_MIN_MIPMAP_LOD,
	DEFAULT_MAX_MIPMAP_LOD,
};
static const Vec2F defaultDepthRange = {
	DEFAULT_MIN_DEPTH,
	DEFAULT_MAX_DEPTH,
};
static const Vec2F defaultDepthBias = {
	DEFAULT_DEPTH_BIAS_CONSTANT,
	DEFAULT_DEPTH_BIAS_SLOPE,
};
static const Vec4F defaultBlendColor = {
	DEFAULT_BLEND_COLOR,
	DEFAULT_BLEND_COLOR,
	DEFAULT_BLEND_COLOR,
	DEFAULT_BLEND_COLOR,
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

typedef enum MouseButton_T
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
} MouseButton_T;

typedef uint8_t MouseButton;

typedef enum CursorMode
{
	DEFAULT_CURSOR_MODE = 0x00034001,
	HIDDEN_CURSOR_MODE = 0x00034002,
	LOCKED_CURSOR_MODE = 0x00034003,
} CursorMode;

typedef enum CursorType_T
{
	DEFAULT_CURSOR_TYPE = 0,
	IBEAM_CURSOR_TYPE = 1,
	CROSSHAIR_CURSOR_TYPE = 2,
	HAND_CURSOR_TYPE = 3,
	HRESIZE_CURSOR_TYPE = 4,
	VRESIZE_CURSOR_TYPE = 5,
	CURSOR_TYPE_COUNT = 6,
} CursorType_T;

typedef uint8_t CursorType;

typedef enum BufferType_T
{
	VERTEX_BUFFER_TYPE = 0b00000001,
	INDEX_BUFFER_TYPE = 0b00000010,
	UNIFORM_BUFFER_TYPE = 0b00000100,
	STORAGE_BUFFER_TYPE = 0b00001000,
	TRANSFER_SOURCE_BUFFER_TYPE = 0b00010000,
	TRANSFER_DESTINATION_BUFFER_TYPE = 0b00100000,
} BufferType_T;

typedef uint8_t BufferType;

typedef enum BufferUsage_T
{
	CPU_ONLY_BUFFER_USAGE = 0,
	GPU_ONLY_BUFFER_USAGE = 1,
	CPU_TO_GPU_BUFFER_USAGE = 2,
	GPU_TO_CPU_BUFFER_USAGE = 3,
	BUFFER_USAGE_COUNT = 4,
} BufferUsage_T;

typedef uint8_t BufferUsage;;

typedef enum ImageDimension_T
{
	IMAGE_1D = 0,
	IMAGE_2D = 1,
	IMAGE_3D = 2,
	IMAGE_DIMENSION_COUNT = 3,
} ImageDimension_T;

typedef uint8_t ImageDimension;

typedef enum ImageType_T
{
	SAMPLED_IMAGE_TYPE = 0b00000001,
	COLOR_ATTACHMENT_IMAGE_TYPE = 0b00000010,
	DEPTH_STENCIL_ATTACHMENT_IMAGE_TYPE = 0b00000100,
	STORAGE_IMAGE_TYPE = 0b00001000,
	TRANSFER_SOURCE_IMAGE_TYPE = 0b00010000,
	TRANSFER_DESTINATION_IMAGE_TYPE = 0b00100000,
} ImageType_T;

typedef uint8_t ImageType;

typedef enum ImageFormat_T
{
	R8_UNORM_IMAGE_FORMAT = 0,
	R8_SRGB_IMAGE_FORMAT = 1,
	R8G8B8A8_UNORM_IMAGE_FORMAT = 2,
	R8G8B8A8_SRGB_IMAGE_FORMAT = 3,
	R16G16B16A16_SFLOAT_IMAGE_FORMAT = 4,
	D16_UNORM_IMAGE_FORMAT = 5,
	D32_SFLOAT_IMAGE_FORMAT = 6,
	D16_UNORM_S8_UINT_IMAGE_FORMAT = 7,
	D24_UNORM_S8_UINT_IMAGE_FORMAT = 8,
	D32_SFLOAT_S8_UINT_IMAGE_FORMAT = 9,
	IMAGE_FORMAT_COUNT = 10,
} ImageFormat_T;

typedef uint8_t ImageFormat;

typedef enum ImageFilter_T
{
	LINEAR_IMAGE_FILTER = 0,
	NEAREST_IMAGE_FILTER = 1,
	IMAGE_FILTER_COUNT = 2,
} ImageFilter_T;

typedef uint8_t ImageFilter;

typedef enum ImageWrap_T
{
	REPEAT_IMAGE_WRAP = 0,
	MIRRORED_REPEAT_IMAGE_WRAP = 1,
	CLAMP_TO_EDGE_IMAGE_WRAP = 2,
	CLAMP_TO_BORDER_IMAGE_WRAP = 3,
	MIRROR_CLAMP_TO_EDGE_IMAGE_WRAP = 4,
	IMAGE_WRAP_COUNT = 5,
} ImageWrap_T;

typedef uint8_t ImageWrap;

typedef enum ShaderType_T
{
	VERTEX_SHADER_TYPE = 0,
	FRAGMENT_SHADER_TYPE = 1,
	COMPUTE_SHADER_TYPE = 2,
	TESSELLATION_CONTROL_SHADER_TYPE = 3,
	TESSELLATION_EVALUATION_SHADER_TYPE = 4,
	GEOMETRY_SHADER_TYPE = 5,
	RAY_GENERATION_SHADER_TYPE = 6,
	RAY_MISS_SHADER_TYPE = 7,
	RAY_CLOSEST_HIT_SHADER_TYPE = 8,
	SHADER_TYPE_COUNT = 9,
	// TODO: any hit, intersection, callable,
	// task mesh shaders
} ShaderType_T;

typedef uint8_t ShaderType;

typedef enum DrawMode_T
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
} DrawMode_T;

typedef uint8_t DrawMode;

typedef enum PolygonMode_T
{
	POINT_POLYGON_MODE = 0,
	LINE_POLYGON_MODE = 1,
	FILL_POLYGON_MODE = 2,
	POLYGON_MODE_COUNT = 3,
} PolygonMode_T;

typedef uint8_t PolygonMode;

typedef enum CullMode_T
{
	FRONT_CULL_MODE = 0,
	BACK_CULL_MODE = 1,
	FRONT_AND_BACK_CULL_MODE = 2,
	CULL_MODE_COUNT = 3,
} CullMode_T;

typedef uint8_t CullMode;

typedef enum CompareOperator_T
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
} CompareOperator_T;

typedef uint8_t CompareOperator;

typedef enum ColorComponent_T
{
	NONE_COLOR_COMPONENT = 0b0000,
	RED_COLOR_COMPONENT = 0b0001,
	GREEN_COLOR_COMPONENT = 0b0010,
	BLUE_COLOR_COMPONENT = 0b0100,
	ALPHA_COLOR_COMPONENT = 0b1000,
	ALL_COLOR_COMPONENT = 0b1111,
} ColorComponent_T;

typedef uint8_t ColorComponent;
#define COLOR_COMPONENT_COUNT 5

typedef enum BlendFactor_T
{
	ZERO_BLEND_FACTOR = 0,
	ONE_BLEND_FACTOR = 1,
	SOURCE_COLOR_BLEND_FACTOR = 2,
	ONE_MINUS_SOURCE_COLOR_BLEND_FACTOR = 3,
	DESTINATION_COLOR_BLEND_FACTOR = 4,
	ONE_MINUS_DESTINATION_COLOR_BLEND_FACTOR = 5,
	SOURCE_ALPHA_BLEND_FACTOR = 6,
	ONE_MINUS_SOURCE_ALPHA_BLEND_FACTOR = 7,
	DESTINATION_ALPHA_BLEND_FACTOR = 8,
	ONE_MINUS_DESTINATION_ALPHA_BLEND_FACTOR = 9,
	CONSTANT_COLOR_BLEND_FACTOR = 10,
	ONE_MINUS_CONSTANT_COLOR_BLEND_FACTOR = 11,
	CONSTANT_ALPHA_BLEND_FACTOR = 12,
	ONE_MINUS_CONSTANT_ALPHA_BLEND_FACTOR = 13,
	SOURCE_ALPHA_SATURATE_BLEND_FACTOR = 14,
	SOURCE_ONE_COLOR_BLEND_FACTOR = 15,
	ONE_MINUS_SOURCE_ONE_COLOR_BLEND_FACTOR = 16,
	SOURCE_ONE_ALPHA_BLEND_FACTOR = 17,
	ONE_MINUS_SOURCE_ONE_ALPHA_BLEND_FACTOR = 18,
	BLEND_FACTOR_COUNT = 19,
} BlendFactor_T;

typedef uint8_t BlendFactor;

typedef enum BlendOperator_T
{
	ADD_BLEND_OPERATOR = 0,
	SUBTRACT_BLEND_OPERATOR = 1,
	REVERSE_SUBTRACT_BLEND_OPERATOR = 2,
	MIN_BLEND_OPERATOR = 3,
	MAX_BLEND_OPERATOR = 4,
	BLEND_OPERATOR_COUNT = 5,
} BlendOperator_T;

typedef uint8_t BlendOperator;

typedef enum IndexType_T
{
	UINT16_INDEX_TYPE = 0,
	UINT32_INDEX_TYPE = 1,
	INDEX_TYPE_COUNT = 2,
} IndexType_T;

typedef uint8_t IndexType;

typedef struct DepthStencilClear
{
	float depth;
	uint32_t stencil;
} DepthStencilClear;
typedef union FramebufferClear
{
	LinearColor color;
	DepthStencilClear depthStencil;
} FramebufferClear;

typedef struct GraphicsPipelineState
{
	DrawMode drawMode;
	PolygonMode polygonMode;
	CullMode cullMode;
	CompareOperator depthCompareOperator;
	ColorComponent colorComponentWriteMask;
	BlendFactor srcColorBlendFactor;
	BlendFactor dstColorBlendFactor;
	BlendFactor srcAlphaBlendFactor;
	BlendFactor dstAlphaBlendFactor;
	BlendOperator colorBlendOperator;
	BlendOperator alphaBlendOperator;
	bool cullFace;
	bool clockwiseFrontFace;
	bool testDepth;
	bool writeDepth;
	bool clampDepth;
	bool enableDepthBias;
	bool enableBlend;
	bool restartPrimitive;
	bool discardRasterizer;
	float lineWidth;
	Vec4I viewport;
	Vec4I scissor;
	Vec2F depthRange;
	Vec2F depthBias;
	Vec4F blendColor;
} GraphicsPipelineState;

typedef struct Window_T Window_T;
typedef Window_T* Window;
typedef union Buffer_T Buffer_T;
typedef Buffer_T* Buffer;
typedef union Image_T Image_T;
typedef Image_T* Image;
typedef union Sampler_T Sampler_T;
typedef Sampler_T* Sampler;
typedef union GraphicsPipeline_T GraphicsPipeline_T;
typedef GraphicsPipeline_T* GraphicsPipeline;
typedef union Framebuffer_T Framebuffer_T;
typedef Framebuffer_T* Framebuffer;
typedef union Shader_T Shader_T;
typedef Shader_T* Shader;
typedef union GraphicsMesh_T GraphicsMesh_T;
typedef GraphicsMesh_T* GraphicsMesh;
typedef union ComputePipeline_T ComputePipeline_T;
typedef ComputePipeline_T* ComputePipeline;
typedef union RayTracingPipeline_T RayTracingPipeline_T;
typedef RayTracingPipeline_T* RayTracingPipeline;
typedef union RayTracingMesh_T RayTracingMesh_T;
typedef RayTracingMesh_T* RayTracingMesh;
typedef union RayTracingScene_T RayTracingScene_T;
typedef RayTracingScene_T* RayTracingScene;

typedef struct ImageData_T ImageData_T;
typedef ImageData_T* ImageData;

typedef void(*OnWindowUpdate)(void* argument);

typedef void(*OnGraphicsPipelineDestroy)(void* handle);
typedef void(*OnGraphicsPipelineBind)(
	GraphicsPipeline graphicsPipeline);
typedef void(*OnGraphicsPipelineUniformsSet)(
	GraphicsPipeline graphicsPipeline);
typedef MpgxResult(*OnGraphicsPipelineResize)(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* createData);

typedef void(*OnComputePipelineDestroy)(void* handle);
typedef void(*OnComputePipelineBind)(
	ComputePipeline computePipeline);

typedef void(*OnRayTracingPipelineDestroy)(void* handle);
typedef void(*OnRayTracingPipelineBind)(
	RayTracingPipeline rayTracingPipeline);

MpgxResult initializeGraphics(
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch);
void terminateGraphics();
bool isGraphicsInitialized();

void* getFtLibrary();

MpgxResult createWindow(
	GraphicsAPI api,
	Vec2I size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool useVerticalSync,
	bool useStencilBuffer,
	bool useBeginClear,
	bool useRayTracing,
	bool isVisible,
	Window* window);
MpgxResult createAnyWindow(
	Vec2I size,
	const char* title,
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool useVerticalSync,
	bool useStencilBuffer,
	bool useBeginClear,
	bool useRayTracing,
	bool isVisible,
	Window* window);
void destroyWindow(Window window);

GraphicsAPI getWindowGraphicsAPI(Window window);
bool isWindowUseVerticalSync(Window window);
bool isWindowUseStencilBuffer(Window window);
bool isWindowUseRayTracing(Window window);
OnWindowUpdate getWindowOnUpdate(Window window);
void* getWindowUpdateArgument(Window window);
const uint32_t* getWindowInputBuffer(Window window);
size_t getWindowInputLength(Window window);
Framebuffer getWindowFramebuffer(Window window);
double getWindowUpdateTime(Window window);
double getWindowDeltaTime(Window window);
Vec2F getWindowContentScale(Window window);
const char* getWindowGpuName(Window window);

void* getVkWindow(Window window);
bool isVkDeviceIntegrated(Window window);

bool getWindowKeyboardKey(
	Window window,
	KeyboardKey key);
bool getWindowMouseButton(
	Window window,
	MouseButton button);

const char* getWindowClipboard(
	Window window);
void setWindowClipboard(
	Window window,
	const char* clipboard);

Vec2I getWindowSize(
	Window window);
void setWindowSize(
	Window window,
	Vec2I size);

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

CursorMode getWindowCursorMode(
	Window window);
void setWindowCursorMode(
	Window window,
	CursorMode cursorMode);

CursorType getWindowCursorType(
	Window window);
void setWindowCursorType(
	Window window,
	CursorType cursorType);

void setWindowTitle(
	Window window,
	const char* title);

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

MpgxResult beginWindowRecord(Window window);
void endWindowRecord(Window window);

// TODO: make create buffer/image/ray mesh batching system. (Vulkan)
// Add option createNow or batch and create with shared cmd buffer

MpgxResult createBuffer(
	Window window,
	BufferType type,
	BufferUsage usage,
	const void* data,
	size_t size,
	Buffer* buffer);
void destroyBuffer(Buffer buffer);

Window getBufferWindow(Buffer buffer);
BufferType getBufferType(Buffer buffer);
BufferUsage getBufferUsage(Buffer buffer);
size_t getBufferSize(Buffer buffer);

MpgxResult mapBuffer(
	Buffer buffer,
	size_t size,
	size_t offset,
	void** map);
MpgxResult unmapBuffer(Buffer buffer);

MpgxResult setBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset);

MpgxResult createImageData(
	const void* data,
	size_t size,
	uint8_t channelCount,
	ImageData* imageData);
MpgxResult createImageDataFromFile(
	const char* filePath,
	uint8_t channelCount,
	ImageData* imageData);
void destroyImageData(ImageData imageData);

const uint8_t* getImageDataPixels(ImageData imageData);
Vec2I getImageDataSize(ImageData imageData);
uint8_t getImageDataChannelCount(ImageData imageData);

MpgxResult createImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3I size,
	uint8_t levelCount,
	bool isConstant,
	Image* image);
MpgxResult createImageFromFile(
	Window window,
	ImageType type,
	ImageFormat format,
	const char* filePath,
	bool generateMipmap,
	bool isConstant,
	Image* image);
MpgxResult createImageFromData(
	Window window,
	ImageType type,
	ImageFormat format,
	const void* data,
	size_t size,
	bool generateMipmap,
	bool isConstant,
	Image* image);
void destroyImage(Image image);

// TODO: set data[].
MpgxResult setImageData(
	Image image,
	const void* data,
	Vec3I size,
	Vec3I offset);

Window getImageWindow(Image image);
ImageType getImageType(Image image);
ImageDimension getImageDimension(Image image);
ImageFormat getImageFormat(Image image);
Vec3I getImageSize(Image image);
bool isImageConstant(Image image);

uint8_t getImageLevelCount(Vec3I imageSize);

MpgxResult createSampler(
	Window window,
	ImageFilter minImageFilter,
	ImageFilter magImageFilter,
	ImageFilter minMipmapFilter,
	bool useMipmapping,
	ImageWrap imageWrapX,
	ImageWrap imageWrapY,
	ImageWrap imageWrapZ,
	CompareOperator compareOperator,
	bool useCompare,
	Vec2F mipmapLodRange,
	float mipmapLodBias,
	Sampler* sampler);
void destroySampler(Sampler sampler);

Window getSamplerWindow(Sampler sampler);
ImageFilter getSamplerMinImageFilter(Sampler sampler);
ImageFilter getSamplerMagImageFilter(Sampler sampler);
ImageFilter getSamplerMinMipmapFilter(Sampler sampler);
bool isSamplerUseMipmapping(Sampler sampler);
ImageWrap getSamplerImageWrapX(Sampler sampler);
ImageWrap getSamplerImageWrapY(Sampler sampler);
ImageWrap getSamplerImageWrapZ(Sampler sampler);
CompareOperator getSamplerCompareOperator(Sampler sampler);
bool isSamplerUseCompare(Sampler sampler);
Vec2F getSamplerMipmapLodRange(Sampler sampler);
float getSamplerMipmapLodBias(Sampler sampler);

MpgxResult createShader(
	Window window,
	ShaderType type,
	const void* code,
	size_t size,
	Shader* shader);
MpgxResult createShaderFromFile(
	Window window,
	ShaderType type,
	const char* filePath,
	Shader* shader);
void destroyShader(Shader shader);

Window getShaderWindow(Shader shader);
ShaderType getShaderType(Shader shader);

// TODO: add deferred rendering framebuffer constructor
// utilize vulkan subpass optimization

MpgxResult createFramebuffer(
	Window window,
	Vec2I size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity,
	Framebuffer* framebuffer);
MpgxResult createShadowFramebuffer(
	Window window,
	Vec2I size,
	bool useBeginClear,
	Image depthAttachment,
	size_t pipelineCapacity,
	Framebuffer* framebuffer);
void destroyFramebuffer(
	Framebuffer framebuffer,
	bool destroyAttachments);

Window getFramebufferWindow(Framebuffer framebuffer);
Vec2I getFramebufferSize(Framebuffer framebuffer);
bool isFramebufferUseBeginClear(Framebuffer framebuffer);
Image* getFramebufferColorAttachments(Framebuffer framebuffer);
size_t getFramebufferColorAttachmentCount(Framebuffer framebuffer);
Image getFramebufferDepthStencilAttachment(Framebuffer framebuffer);
bool isFramebufferDefault(Framebuffer framebuffer);

MpgxResult setFramebufferAttachments(
	Framebuffer framebuffer,
	Vec2I size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment);

void beginFramebufferRender(
	Framebuffer framebuffer,
	const FramebufferClear* clearValues,
	size_t clearValueCount);
void endFramebufferRender(
	Framebuffer framebuffer);

void clearFramebuffer(
	Framebuffer framebuffer,
	const bool* clearAttachments,
	const FramebufferClear* clearValues,
	size_t clearValueCount);

MpgxResult createGraphicsPipeline(
	Framebuffer framebuffer,
	const char* name,
	const GraphicsPipelineState* state,
	OnGraphicsPipelineBind onBind,
	OnGraphicsPipelineUniformsSet onUniformsSet,
	OnGraphicsPipelineResize onResize,
	OnGraphicsPipelineDestroy onDestroy,
	void* handle,
	const void* createData,
	Shader* shaders,
	size_t shaderCount,
	GraphicsPipeline* graphicsPipeline);
void destroyGraphicsPipeline(
	GraphicsPipeline graphicsPipeline,
	bool destroyShaders);

Framebuffer getGraphicsPipelineFramebuffer(
	GraphicsPipeline graphicsPipeline);
const char* getGraphicsPipelineName(
	GraphicsPipeline graphicsPipeline);
const GraphicsPipelineState* getGraphicsPipelineState(
	GraphicsPipeline graphicsPipeline);
OnGraphicsPipelineBind getGraphicsPipelineOnBind(
	GraphicsPipeline graphicsPipeline);
OnGraphicsPipelineUniformsSet getGraphicsPipelineOnUniformsSet(
	GraphicsPipeline graphicsPipeline);
OnGraphicsPipelineResize getGraphicsPipelineOnResize(
	GraphicsPipeline graphicsPipeline);
OnGraphicsPipelineDestroy getGraphicsPipelineOnDestroy(
	GraphicsPipeline graphicsPipeline);
void* getGraphicsPipelineHandle(
	GraphicsPipeline graphicsPipeline);
Shader* getGraphicsPipelineShaders(
	GraphicsPipeline graphicsPipeline);
size_t getGraphicsPipelineShaderCount(
	GraphicsPipeline graphicsPipeline);
Window getGraphicsPipelineWindow(
	GraphicsPipeline graphicsPipeline);

void bindGraphicsPipeline(
	GraphicsPipeline graphicsPipeline);

MpgxResult createGraphicsMesh(
	Window window,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	GraphicsMesh* graphicsMesh);
void destroyGraphicsMesh(
	GraphicsMesh graphicsMesh,
	bool destroyBuffers);

Window getGraphicsMeshWindow(GraphicsMesh graphicsMesh);
IndexType getGraphicsMeshIndexType(GraphicsMesh graphicsMesh);

size_t getGraphicsMeshIndexCount(
	GraphicsMesh graphicsMesh);
void setGraphicsMeshIndexCount(
	GraphicsMesh graphicsMesh,
	size_t indexCount);

size_t getGraphicsMeshIndexOffset(
	GraphicsMesh graphicsMesh);
void setGraphicsMeshIndexOffset(
	GraphicsMesh graphicsMesh,
	size_t indexOffset);

Buffer getGraphicsMeshVertexBuffer(
	GraphicsMesh graphicsMesh);
void setGraphicsMeshVertexBuffer(
	GraphicsMesh graphicsMesh,
	Buffer vertexBuffer);

Buffer getGraphicsMeshIndexBuffer(
	GraphicsMesh graphicsMesh);
void setGraphicsMeshIndexBuffer(
	GraphicsMesh graphicsMesh,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer indexBuffer);

size_t drawGraphicsMesh(
	GraphicsPipeline graphicsPipeline,
	GraphicsMesh graphicsMesh);

MpgxResult createComputePipeline(
	Window window,
	const char* name,
	OnComputePipelineBind onBind,
	OnComputePipelineDestroy onDestroy,
	void* handle,
	const void* createData,
	Shader shader,
	ComputePipeline* computePipeline);
void destroyComputePipeline(
	ComputePipeline computePipeline,
	bool destroyShader);

Window getComputePipelineWindow(
	ComputePipeline computePipeline);
const char* getComputePipelineName(
	ComputePipeline computePipeline);
OnComputePipelineBind getComputePipelineOnBind(
	ComputePipeline computePipeline);
OnComputePipelineDestroy getComputePipelineOnDestroy(
	ComputePipeline computePipeline);
Shader getComputePipelineShader(
	ComputePipeline computePipeline);
void* getComputePipelineHandle(
	ComputePipeline computePipeline);

void bindComputePipeline(
	ComputePipeline computePipeline);

void dispatchComputePipeline(
	ComputePipeline computePipeline,
	size_t groupCountX,
	size_t groupCountY,
	size_t groupCountZ);

MpgxResult createRayTracingPipeline(
	Window window,
	const char* name,
	OnRayTracingPipelineBind onBind,
	OnRayTracingPipelineDestroy onDestroy,
	void* handle,
	const void* createData,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount,
	RayTracingPipeline* rayTracingPipeline);
void destroyRayTracingPipeline(
	RayTracingPipeline rayTracingPipeline,
	bool destroyShaders);

Window getRayTracingPipelineWindow(
	RayTracingPipeline rayTracingPipeline);
const char* getRayTracingPipelineName(
	RayTracingPipeline rayTracingPipeline);
OnRayTracingPipelineBind getRayTracingPipelineOnBind(
	RayTracingPipeline rayTracingPipeline);
OnRayTracingPipelineDestroy getRayTracingPipelineOnDestroy(
	RayTracingPipeline rayTracingPipeline);
void* getRayTracingPipelineHandle(
	RayTracingPipeline rayTracingPipeline);
Shader* getRayTracingPipelineGenerationShaders(
	RayTracingPipeline rayTracingPipeline);
size_t getRayTracingPipelineGenerationShaderCount(
	RayTracingPipeline rayTracingPipeline);
Shader* getRayTracingPipelineMissShaders(
	RayTracingPipeline rayTracingPipeline);
size_t getRayTracingPipelineMissShaderCount(
	RayTracingPipeline rayTracingPipeline);
Shader* getRayTracingPipelineClosestHitShaders(
	RayTracingPipeline rayTracingPipeline);
size_t getRayTracingPipelineClosestHitShaderCount(
	RayTracingPipeline rayTracingPipeline);

void bindRayTracingPipeline(RayTracingPipeline rayTracingPipeline);
void traceRayTracingPipeline(RayTracingPipeline rayTracingPipeline);

MpgxResult createRayTracingMesh(
	Window window,
	size_t vertexStride,
	IndexType indexType,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	RayTracingMesh* rayTracingMesh);
void destroyRayTracingMesh(
	RayTracingMesh rayTracingMesh,
	bool destroyBuffers);

Window getRayTracingMeshWindow(RayTracingMesh rayTracingMesh);
size_t getRayTracingMeshVertexStride(RayTracingMesh rayTracingMesh);
IndexType getRayTracingMeshIndexType(RayTracingMesh rayTracingMesh);
Buffer getRayTracingMeshVertexBuffer(RayTracingMesh rayTracingMesh);
Buffer getRayTracingMeshIndexBuffer(RayTracingMesh rayTracingMesh);

// TODO: get/set ray mesh transform matrix

MpgxResult createRayTracingScene(
	Window window,
	RayTracingMesh* meshes,
	size_t meshCount,
	RayTracingScene* rayTracingScene);
void destroyRayTracingScene(
	RayTracingScene rayTracingScene);

Window getRayTracingSceneWindow(RayTracingScene rayTracingScene);
RayTracingMesh* getRayTracingSceneMeshes(RayTracingScene rayTracingScene);
size_t getRayTracingSceneMeshCount(RayTracingScene rayTracingScene);

// TODO: add/remove ray scene mesh

// TODO: add functions:
// create group: buffer, image, ray mesh
