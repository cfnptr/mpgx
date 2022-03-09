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

#define DEFAULT_WINDOW_TITLE "MPGX (v" MPGX_VERSION_STRING ")"
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

// TODO: add ability to store and load shader cache
// TODO: add window item enumerators and count getters
// TODO: include static vulkan library on MacOS
// TODO: add buffer/image array creation function with shared resources.
// TODO: set buffer and image multiple data arrays in one call
// TODO: add any hit, intersection, callable, task mesh shaders
// TODO: add/remove ray scene mesh
// TODO: get/set ray mesh transform matrix
// TODO: add hash checking system for the images and pipelines.
// TODO: add deferred rendering framebuffer constructor, utilize vulkan subpass optimization

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

/*
 * Keyboard key types.
 */
typedef enum KeyboardKey_T
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
} KeyboardKey_T;
/*
 * Keyboard key type.
 */
typedef int KeyboardKey;

/*
 * Mouse button types.
 */
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
/*
 * Mouse button type.
 */
typedef int MouseButton;

/*
 * Cursor modes.
 */
typedef enum CursorMode_T
{
	DEFAULT_CURSOR_MODE = 0x00034001,
	HIDDEN_CURSOR_MODE = 0x00034002,
	LOCKED_CURSOR_MODE = 0x00034003,
} CursorMode_T;
/*
 * Cursor mode mask.
 */
typedef int CursorMode;

/*
 * Cursor types.
 */
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
/*
 * Cursor type.
 */
typedef uint8_t CursorType;

/*
 * Buffer types.
 */
typedef enum BufferType_T
{
	VERTEX_BUFFER_TYPE = 0b00000001,
	INDEX_BUFFER_TYPE = 0b00000010,
	UNIFORM_BUFFER_TYPE = 0b00000100,
	STORAGE_BUFFER_TYPE = 0b00001000,
	TRANSFER_SOURCE_BUFFER_TYPE = 0b00010000,
	TRANSFER_DESTINATION_BUFFER_TYPE = 0b00100000,
} BufferType_T;
/*
 * Buffer type mask.
 */
typedef uint8_t BufferType;

/*
 * Buffer usage types.
 */
typedef enum BufferUsage_T
{
	CPU_ONLY_BUFFER_USAGE = 0,
	GPU_ONLY_BUFFER_USAGE = 1,
	CPU_TO_GPU_BUFFER_USAGE = 2,
	GPU_TO_CPU_BUFFER_USAGE = 3,
	BUFFER_USAGE_COUNT = 4,
} BufferUsage_T;
/*
 * Buffer usage type.
 */
typedef uint8_t BufferUsage;

/*
 * Image dimension types.
 */
typedef enum ImageDimension_T
{
	IMAGE_1D = 0,
	IMAGE_2D = 1,
	IMAGE_3D = 2,
	IMAGE_DIMENSION_COUNT = 3,
} ImageDimension_T;
/*
 * Image dimension type.
 */
typedef uint8_t ImageDimension;

/*
 * Image types.
 */
typedef enum ImageType_T
{
	SAMPLED_IMAGE_TYPE = 0b00000001,
	COLOR_ATTACHMENT_IMAGE_TYPE = 0b00000010,
	DEPTH_STENCIL_ATTACHMENT_IMAGE_TYPE = 0b00000100,
	STORAGE_IMAGE_TYPE = 0b00001000,
	TRANSFER_SOURCE_IMAGE_TYPE = 0b00010000,
	TRANSFER_DESTINATION_IMAGE_TYPE = 0b00100000,
} ImageType_T;
/*
 * Image type mask.
 */
typedef uint8_t ImageType;

/*
 * Image format types.
 */
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
/*
 * Image format type.
 */
typedef uint8_t ImageFormat;

/*
 * Image filter types.
 */
typedef enum ImageFilter_T
{
	LINEAR_IMAGE_FILTER = 0,
	NEAREST_IMAGE_FILTER = 1,
	IMAGE_FILTER_COUNT = 2,
} ImageFilter_T;
/*
 * Image filter type.
 */
typedef uint8_t ImageFilter;

/*
 * Image wrap types.
 */
typedef enum ImageWrap_T
{
	REPEAT_IMAGE_WRAP = 0,
	MIRRORED_REPEAT_IMAGE_WRAP = 1,
	CLAMP_TO_EDGE_IMAGE_WRAP = 2,
	CLAMP_TO_BORDER_IMAGE_WRAP = 3,
	MIRROR_CLAMP_TO_EDGE_IMAGE_WRAP = 4,
	IMAGE_WRAP_COUNT = 5,
} ImageWrap_T;
/*
 * Image wrap type.
 */
typedef uint8_t ImageWrap;

/*
 * Shader types.
 */
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
} ShaderType_T;
/*
 * Shader type.
 */
typedef uint8_t ShaderType;

/*
 * Draw mode types.
 */
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
/*
 * Draw mode type.
 */
typedef uint8_t DrawMode;

/*
 * Polygon mode types.
 */
typedef enum PolygonMode_T
{
	POINT_POLYGON_MODE = 0,
	LINE_POLYGON_MODE = 1,
	FILL_POLYGON_MODE = 2,
	POLYGON_MODE_COUNT = 3,
} PolygonMode_T;
/*
 * Polygon mode type.
 */
typedef uint8_t PolygonMode;

/*
 * Cull mode types.
 */
typedef enum CullMode_T
{
	FRONT_CULL_MODE = 0,
	BACK_CULL_MODE = 1,
	FRONT_AND_BACK_CULL_MODE = 2,
	CULL_MODE_COUNT = 3,
} CullMode_T;
/*
 * Cull mode type.
 */
typedef uint8_t CullMode;

/*
 * Compare operator types.
 */
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
/*
 * Compare operator type.
 */
typedef uint8_t CompareOperator;

/*
 * Color component types.
 */
typedef enum ColorComponent_T
{
	NONE_COLOR_COMPONENT = 0b0000,
	RED_COLOR_COMPONENT = 0b0001,
	GREEN_COLOR_COMPONENT = 0b0010,
	BLUE_COLOR_COMPONENT = 0b0100,
	ALPHA_COLOR_COMPONENT = 0b1000,
	ALL_COLOR_COMPONENT = 0b1111,
} ColorComponent_T;
/*
 * Color component mask.
 */
typedef uint8_t ColorComponent;
#define COLOR_COMPONENT_COUNT 5

/*
 * Blend factor types.
 */
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
/*
 * Blend factor type.
 */
typedef uint8_t BlendFactor;

/*
 * Blend operator types.
 */
typedef enum BlendOperator_T
{
	ADD_BLEND_OPERATOR = 0,
	SUBTRACT_BLEND_OPERATOR = 1,
	REVERSE_SUBTRACT_BLEND_OPERATOR = 2,
	MIN_BLEND_OPERATOR = 3,
	MAX_BLEND_OPERATOR = 4,
	BLEND_OPERATOR_COUNT = 5,
} BlendOperator_T;

/*
 * Blend operator type.
 */
typedef uint8_t BlendOperator;

/*
 * Buffer index types.
 */
typedef enum IndexType_T
{
	UINT16_INDEX_TYPE = 0,
	UINT32_INDEX_TYPE = 1,
	INDEX_TYPE_COUNT = 2,
} IndexType_T;
/*
 * Buffer index type.
 */
typedef uint8_t IndexType;

/*
 * Framebuffer depth stencil clear data structure.
 */
typedef struct DepthStencilClear
{
	float depth;
	uint32_t stencil;
} DepthStencilClear;
/*
 * Framebuffer clear data structure.
 */
typedef union FramebufferClear
{
	LinearColor color;
	DepthStencilClear depthStencil;
} FramebufferClear;

/*
 * Graphics pipeline state structure.
 */
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

/*
 * Window structure.
 */
typedef struct Window_T Window_T;
/*
 * Window instance.
 */
typedef Window_T* Window;
/*
 * Buffer structure.
 */
typedef union Buffer_T Buffer_T;
/*
 * Buffer instance.
 */
typedef Buffer_T* Buffer;
/*
 * Image structure.
 */
typedef union Image_T Image_T;
/*
 * Image instance.
 */
typedef Image_T* Image;
/*
 * Sampler structure.
 */
typedef union Sampler_T Sampler_T;
/*
 * Sampler instance.
 */
typedef Sampler_T* Sampler;
/*
 * Graphics pipeline structure.
 */
typedef union GraphicsPipeline_T GraphicsPipeline_T;
/*
 * Graphics pipeline instance.
 */
typedef GraphicsPipeline_T* GraphicsPipeline;
/*
 * Framebuffer structure.
 */
typedef union Framebuffer_T Framebuffer_T;
/*
 * Framebuffer instance.
 */
typedef Framebuffer_T* Framebuffer;
/*
 * Shader structure.
 */
typedef union Shader_T Shader_T;
/*
 * Shader instance.
 */
typedef Shader_T* Shader;
/*
 * Graphics mesh structure.
 */
typedef union GraphicsMesh_T GraphicsMesh_T;
/*
 * Graphics mesh instance.
 */
typedef GraphicsMesh_T* GraphicsMesh;
/*
 * Compute pipeline structure.
 */
typedef union ComputePipeline_T ComputePipeline_T;
/*
 * Compute pipeline instance.
 */
typedef ComputePipeline_T* ComputePipeline;
/*
 * Ray tracing pipeline structure.
 */
typedef union RayTracingPipeline_T RayTracingPipeline_T;
/*
 * Ray tracing pipeline instance.
 */
typedef RayTracingPipeline_T* RayTracingPipeline;
/*
 * Ray tracing mesh structure.
 */
typedef union RayTracingMesh_T RayTracingMesh_T;
/*
 * Ray tracing mesh instance.
 */
typedef RayTracingMesh_T* RayTracingMesh;
/*
 * Ray tracing scene structure.
 */
typedef union RayTracingScene_T RayTracingScene_T;
/*
 * Ray tracing scene instance.
 */
typedef RayTracingScene_T* RayTracingScene;

/*
 * Window update function.
 */
typedef void(*OnWindowUpdate)(void* argument);

/*
 * Graphics pipeline destroy function.
 */
typedef void(*OnGraphicsPipelineDestroy)(void* handle);
/*
 * Graphics pipeline bind function.
 */
typedef void(*OnGraphicsPipelineBind)(
	GraphicsPipeline graphicsPipeline);
/*
 * Graphics pipeline uniforms set function.
 */
typedef void(*OnGraphicsPipelineUniformsSet)(
	GraphicsPipeline graphicsPipeline);
/*
 * Graphics pipeline resize function.
 */
typedef MpgxResult(*OnGraphicsPipelineResize)(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* createData);

/*
 * Compute pipeline destroy function.
 */
typedef void(*OnComputePipelineDestroy)(void* handle);
/*
 * Compute pipeline bind function.
 */
typedef void(*OnComputePipelineBind)(
	ComputePipeline computePipeline);

/*
 * Ray tracing pipeline destroy function.
 */
typedef void(*OnRayTracingPipelineDestroy)(void* handle);
/*
 * Ray tracing pipeline bind function.
 */
typedef void(*OnRayTracingPipelineBind)(
	RayTracingPipeline rayTracingPipeline);

/*
 * Initialize graphics subsystems.
 * Returns operation MPGX result.
 *
 * Provided information can be used to optimize rendering.
 *
 * engineName - engine name string.
 * engineVersionMajor - major engine version.
 * engineVersionMinor - minor engine version.
 * engineVersionPatch - patch engine version.
 * appName - application name string.
 * appVersionMajor - major application version.
 * appVersionMinor - minor application version.
 * appVersionPatch - patch application version.
 */
MpgxResult initializeGraphics(
	GraphicsAPI api,
	const char* engineName,
	uint8_t engineVersionMajor,
	uint8_t engineVersionMinor,
	uint8_t engineVersionPatch,
	const char* appName,
	uint8_t appVersionMajor,
	uint8_t appVersionMinor,
	uint8_t appVersionPatch);
/*
 * Terminates graphics subsystems.
 */
void terminateGraphics();
/*
 * Returns true if graphics subsystems are initialized.
 */
bool isGraphicsInitialized();
/*
 * Returns graphics subsystem API.
 */
GraphicsAPI getGraphicsAPI();

/*
 * Create a new window instance.
 * Returns operation MPGX result.
 *
 * onUpdate - on window update function.
 * updateArgument - onUpdate function argument.
 * useStencilBuffer - use stencil buffer in the framebuffer.
 * useRayTracing - use ray tracing extension.
 * parent - window parent or NULL.
 * window - pointer to the window.
 */
MpgxResult createWindow(
	OnWindowUpdate onUpdate,
	void* updateArgument,
	bool useStencilBuffer,
	bool useBeginClear,
	bool useRayTracing,
	Window parent,
	Window* window);
/*
 * Destroys window instance.
 * window - pointer to the window or NULL.
 */
void destroyWindow(Window window);

/*
 * Returns window parent.
 * window - window instance.
 */
Window getWindowParent(Window window);
/*
 * Returns true if window uses stencil buffer.
 * window - window instance.
 */
bool isWindowUseStencilBuffer(Window window);
/*
 * Returns true if window uses ray tracing extension.
 * window - window instance.
 */
bool isWindowUseRayTracing(Window window);
/*
 * Returns window on update function.
 * window - window instance.
 */
OnWindowUpdate getWindowOnUpdate(Window window);
/*
 * Returns window on update function argument.
 * window - window instance.
 */
void* getWindowUpdateArgument(Window window);
/*
 * Returns current frame window input character buffer.
 * window - window instance.
 */
const uint32_t* getWindowInputBuffer(Window window);
/*
 * Returns current window frame character
 * count in the input buffer.
 *
 * window - window instance.
 */
size_t getWindowInputLength(Window window);
/*
 * Returns default window framebuffer.
 * window - window instance.
 */
Framebuffer getWindowFramebuffer(Window window);
/*
 * Returns current window frame time.
 * window - window instance.
 */
double getWindowUpdateTime(Window window);
/*
 * Returns current window frame delta time.
 * window - window instance.
 */
double getWindowDeltaTime(Window window);
/*
 * Returns window content scale.
 * window - window instance.
 */
Vec2F getWindowContentScale(Window window);
/*
 * Returns window GPU name string.
 * window - window instance.
 */
const char* getWindowGpuName(Window window);
/*
 * Returns window GPU driver string.
 * window - window instance.
 */
const char* getWindowGpuDriver(Window window);

/*
 * Returns Vulkan window instance.
 * window - window instance.
 */
void* getVkWindow(Window window);
/*
 * Returns true if Vulkan window device is integrated.
 * window - window instance.
 */
bool isVkDeviceIntegrated(Window window);

/*
 * Returns true if specified window keyboard key is pressed.
 *
 * window - window instance.
 * key - keyboard key type.
 */
bool getWindowKeyboardKey(
	Window window,
	KeyboardKey key);
/*
 * Returns true if specified window mouse button is pressed.
 *
 * window - window instance.
 * button - mouse button type.
 */
bool getWindowMouseButton(
	Window window,
	MouseButton button);

/*
 * Returns true if window use VSync.
 * window - window instance.
 */
bool isWindowUseVsync(
	Window window);
/*
 * Sets window VSync use value.
 *
 * window - window instance.
 * useVsync - use VSync value.
 */
void setWindowUseVsync(
	Window window,
	bool useVsync);

/*
 * Returns current window clipboard data.
 * window - window instance.
 */
const char* getWindowClipboard(
	Window window);
/*
 * Sets window clipboard data.
 *
 * window - window instance.
 * clipboard - clipboard data.
 */
void setWindowClipboard(
	Window window,
	const char* clipboard);

/*
 * Returns current window size.
 * window - window instance.
 */
Vec2I getWindowSize(
	Window window);
/*
 * Sets window size.
 *
 * window - window instance.
 * size - window size.
 */
void setWindowSize(
	Window window,
	Vec2I size);

/*
 * Returns current window position.
 * window - window instance.
 */
Vec2I getWindowPosition(
	Window window);
/*
 * Sets window position.
 *
 * window - window instance.
 * position - window position.
 */
void setWindowPosition(
	Window window,
	Vec2I position);

/*
 * Returns current window cursor position.
 * window - window instance.
 */
Vec2F getWindowCursorPosition(
	Window window);
/*
 * Sets window cursor position.
 *
 * window - window instance.
 * position - cursor position.
 */
void setWindowCursorPosition(
	Window window,
	Vec2F position);

/*
 * Returns window cursor mode.
 * window - window instance.
 */
CursorMode getWindowCursorMode(
	Window window);
/*
 * Sets window cursor mode.
 *
 * window - window instance.
 * mode - cursor mode type.
 */
void setWindowCursorMode(
	Window window,
	CursorMode mode);

/*
 * Returns window cursor type.
 * window - window instance.
 */
CursorType getWindowCursorType(
	Window window);
/*
 * Sets window cursor type.
 *
 * window - window instance.
 * type - cursor type.
 */
void setWindowCursorType(
	Window window,
	CursorType type);

/*
 * Sets window title.
 *
 * window - window instance.
 * type - window title string.
 */
void setWindowTitle(
	Window window,
	const char* title);

/*
 * Returns true if window is focused.
 * window - window instance.
 */
bool isWindowFocused(Window window);
/*
 * Returns true if window is iconified.
 * window - window instance.
 */
bool isWindowIconified(Window window);
/*
 * Returns true if window is maximized.
 * window - window instance.
 */
bool isWindowMaximized(Window window);
/*
 * Returns true if window is visible.
 * window - window instance.
 */
bool isWindowVisible(Window window);
/*
 * Returns true if window is hovered.
 * window - window instance.
 */
bool isWindowHovered(Window window);

/*
 * Iconifies window.
 * window - window instance.
 */
void iconifyWindow(Window window);
/*
 * Maximizes window.
 * window - window instance.
 */
void maximizeWindow(Window window);
/*
 * Restores window.
 * window - window instance.
 */
void restoreWindow(Window window);
/*
 * Shows window.
 * window - window instance.
 */
void showWindow(Window window);
/*
 * Hides window.
 * window - window instance.
 */
void hideWindow(Window window);
/*
 * Focuses window.
 * window - window instance.
 */
void focusWindow(Window window);
/*
 * Request window attention window.
 * window - window instance.
 */
void requestWindowAttention(Window window);

/*
 * Makes OpenGL window context current.
 * window - window instance.
 */
void makeGlWindowContextCurrent(Window window);
/*
 * Joins window update loop.
 * window - window instance.
 */
void joinWindow(Window window);

/*
 * Begins window rendering command record.
 * window - window instance.
 */
MpgxResult beginWindowRecord(Window window);
/*
 * Ends window rendering command record.
 * window - window instance.
 */
void endWindowRecord(Window window);

/*
 * Create a new buffer instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * type - type mask.
 * usage - usage type.
 * data - binary data or NULL.
 * size - buffer size in bytes.
 * buffer - pointer to the buffer instance.
 */
MpgxResult createBuffer(
	Window window,
	BufferType type,
	BufferUsage usage,
	const void* data,
	size_t size,
	Buffer* buffer);
/*
 * Destroys buffer instance.
 * buffer - buffer instance or NULL.
 */
void destroyBuffer(Buffer buffer);

/*
 * Returns buffer window instance.
 * buffer - buffer instance.
 */
Window getBufferWindow(Buffer buffer);
/*
 * Returns buffer type mask.
 * buffer - buffer instance.
 */
BufferType getBufferType(Buffer buffer);
/*
 * Returns buffer usage type.
 * buffer - buffer instance.
 */
BufferUsage getBufferUsage(Buffer buffer);
/*
 * Returns buffer size in bytes.
 * buffer - buffer instance.
 */
size_t getBufferSize(Buffer buffer);

/*
 * Map buffer instance memory.
 * Returns operation MPGX result.
 *
 * buffer - buffer instance.
 * size - map size in bytes.
 * offset - map offset in bytes or 0.
 * map - pointer to the map.
 */
MpgxResult mapBuffer(
	Buffer buffer,
	size_t size,
	size_t offset,
	void** map);
/*
 * Unmap buffer instance memory.
 * Returns operation MPGX result.
 *
 * buffer - buffer instance.
 */
MpgxResult unmapBuffer(Buffer buffer);

/*
 * Set buffer data.
 * Returns operation MPGX result.
 *
 * buffer - buffer instance.
 * data - data array.
 * size - data size in bytes.
 * size - buffer data offset in bytes or 0.
 */
MpgxResult setBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset);

/*
 * Create a new mipmap image instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * type - type mask
 * dimension - dimension type.
 * format - format type.
 * data - mipmap pixel data array.
 * size - image size in pixels.
 * mipCount - mipmap level count.
 * layerCount - array layer count.
 * isConstant - is image constant.
 * image - pointer to the image.
 */
MpgxResult createMipmapImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3I size,
	uint32_t mipCount,
	uint32_t layerCount,
	bool isConstant,
	Image* image);
/*
 * Create a new image instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * type - type mask
 * dimension - dimension type.
 * format - format type.
 * data - pixel data.
 * size - image size in pixels.
 * mipCount - mipmap level count.
 * layerCount - array layer count.
 * isConstant - is image constant.
 * image - pointer to the image.
 */
MpgxResult createImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void* data,
	Vec3I size,
	uint32_t layerCount,
	bool isConstant,
	Image* image);
/*
 * Destroys image instance.
 * image - image instance or NULL.
 */
void destroyImage(Image image);

/*
 * Set mipmap image pixel data.
 * Returns operation MPGX result.
 *
 * image - image instance.
 * data - mipmap pixel data array.
 * size - data size in pixels.
 * offset - data offset in pixels or 0.
 */
MpgxResult setMipmapImageData(
	Image image,
	const void** data,
	Vec3I size,
	Vec3I offset);
/*
 * Set image pixel data.
 * Returns operation MPGX result.
 *
 * image - image instance.
 * data - pixel data.
 * size - data size in pixels.
 * offset - data offset in pixels or 0.
 * mipLevel - mipmap level index.
 */
MpgxResult setImageData(
	Image image,
	const void* data,
	Vec3I size,
	Vec3I offset,
	uint8_t mipLevel);

/*
 * Returns image window instance.
 * image - image instance.
 */
Window getImageWindow(Image image);
/*
 * Returns image type mask.
 * image - image instance.
 */
ImageType getImageType(Image image);
/*
 * Returns image dimension type.
 * image - image instance.
 */
ImageDimension getImageDimension(Image image);
/*
 * Returns image format type.
 * image - image instance.
 */
ImageFormat getImageFormat(Image image);
/*
 * Returns image mipmap level count.
 * image - image instance.
 */
uint32_t getImageMipCount(Image image);
/*
 * Returns image array layer count.
 * image - image instance.
 */
uint32_t getImageLayerCount(Image image);
/*
 * Returns image size in pixels.
 * image - image instance.
 */
Vec3I getImageSize(Image image);
/*
 * Returns true if image instance window.
 * image - image instance.
 */
bool isImageConstant(Image image);

/*
 * Calculates image mip level count based on size.
 * size - image size in pixels.
 */
inline static uint8_t calcMipLevelCount(Vec3I size)
{
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	uint32_t value = max(max(size.x, size.y), size.z);
	return (uint8_t)floor(log2((double)value)) + 1;
}

/*
 * Create a new sampler instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * minImageFilter - minification image filter type.
 * magImageFilter - magnification image filter type.
 * minMipmapFilter - minification mipmap filter type.
 * useMipmapping - use image mipmapping.
 * imageWrapX - X-axis image wrap type.
 * imageWrapY - Y-axis image wrap type.
 * imageWrapZ - Z-axis image wrap type.
 * depthCompare - depth compare operator type.
 * useCompare - use image depth compare.
 * mipmapLodRange - image mipmap level of detail range.
 * mipmapLodBias - image mipmap level of detail bias.
 * sampler - pointer to the sampler instance.
 */
MpgxResult createSampler(
	Window window,
	ImageFilter minImageFilter,
	ImageFilter magImageFilter,
	ImageFilter minMipmapFilter,
	bool useMipmapping,
	ImageWrap imageWrapX,
	ImageWrap imageWrapY,
	ImageWrap imageWrapZ,
	CompareOperator depthCompare,
	bool useCompare,
	Vec2F mipmapLodRange,
	float mipmapLodBias,
	Sampler* sampler);
/*
 * Destroys sampler instance.
 * sampler - sampler instance or NULL.
 */
void destroySampler(Sampler sampler);

/*
 * Returns sampler window instance.
 * sampler - sampler instance.
 */
Window getSamplerWindow(Sampler sampler);
/*
 * Returns sampler minification image filter type.
 * sampler - sampler instance.
 */
ImageFilter getSamplerMinImageFilter(Sampler sampler);
/*
 * Returns sampler magnification image filter type.
 * sampler - sampler instance.
 */
ImageFilter getSamplerMagImageFilter(Sampler sampler);
/*
 * Returns sampler minification mipmap filter type.
 * sampler - sampler instance.
 */
ImageFilter getSamplerMinMipmapFilter(Sampler sampler);
/*
 * Returns true if sampler use mipmapping.
 * sampler - sampler instance.
 */
bool isSamplerUseMipmapping(Sampler sampler);
/*
 * Returns sampler X-axis image wrap type.
 * sampler - sampler instance.
 */
ImageWrap getSamplerImageWrapX(Sampler sampler);
/*
 * Returns sampler Y-axis image wrap type.
 * sampler - sampler instance.
 */
ImageWrap getSamplerImageWrapY(Sampler sampler);
/*
 * Returns sampler Z-axis image wrap type.
 * sampler - sampler instance.
 */
ImageWrap getSamplerImageWrapZ(Sampler sampler);
/*
 * Returns sampler depth compare operator type.
 * sampler - sampler instance.
 */
CompareOperator getSamplerDepthCompare(Sampler sampler);
/*
 * Returns true if sampler is use depth compare.
 * sampler - sampler instance.
 */
bool isSamplerUseCompare(Sampler sampler);
/*
 * Returns sampler mipmap level of detail range.
 * sampler - sampler instance.
 */
Vec2F getSamplerMipmapLodRange(Sampler sampler);
/*
 * Returns sampler mipmap level of detail bias.
 * sampler - sampler instance.
 */
float getSamplerMipmapLodBias(Sampler sampler);

/*
 * Create a new shader instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * type - shader type.
 * code - shader program code.
 * size - code size in bytes.
 * shader - pointer to the shader instance.
 */
MpgxResult createShader(
	Window window,
	ShaderType type,
	const void* code,
	size_t size,
	Shader* shader);
/*
 * Destroys shader instance.
 * shader - shader instance or NULL.
 */
void destroyShader(Shader shader);

/*
 * Returns shader window instance.
 * shader - shader instance.
 */
Window getShaderWindow(Shader shader);
/*
 * Returns shader type
 * shader - shader instance.
 */
ShaderType getShaderType(Shader shader);

/*
 * Create a new framebuffer instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * size - framebuffer size in pixels.
 * useBeginClear - use begin function clear values.
 * colorAttachments - color attachment instance array or NULL.
 * colorAttachmentCount - color attachment count or 0.
 * depthStencilAttachment - depth/stencil attachment instance or NULL.
 * capacity - initial pipeline array capacity or 0.
 * framebuffer - pointer to the framebuffer instance.
 */
MpgxResult createFramebuffer(
	Window window,
	Vec2I size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t capacity,
	Framebuffer* framebuffer);
/*
 * Create a new shadow framebuffer instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * size - framebuffer size in pixels.
 * useBeginClear - use begin function clear values.
 * depthAttachment - depth attachment instance.
 * pipelineCapacity - initial pipeline array capacity or 0.
 * framebuffer - pointer to the framebuffer instance.
 */
MpgxResult createShadowFramebuffer(
	Window window,
	Vec2I size,
	bool useBeginClear,
	Image depthAttachment,
	size_t capacity,
	Framebuffer* framebuffer);
/*
 * Destroys framebuffer instance.
 * framebuffer - framebuffer instance or NULL.
 */
void destroyFramebuffer(Framebuffer framebuffer);

/*
 * Returns framebuffer window instance.
 * framebuffer - framebuffer instance.
 */
Window getFramebufferWindow(Framebuffer framebuffer);
/*
 * Returns framebuffer size in pixels.
 * framebuffer - framebuffer instance.
 */
Vec2I getFramebufferSize(Framebuffer framebuffer);
/*
 * Returns true if framebuffer is use begin function clear values.
 * framebuffer - framebuffer instance.
 */
bool isFramebufferUseBeginClear(Framebuffer framebuffer);
/*
 * Returns framebuffer color attachment array.
 * framebuffer - framebuffer instance.
 */
Image* getFramebufferColorAttachments(Framebuffer framebuffer);
/*
 * Returns framebuffer color attachment count.
 * framebuffer - framebuffer instance.
 */
size_t getFramebufferColorAttachmentCount(Framebuffer framebuffer);
/*
 * Returns framebuffer depth/stencil attachment instance.
 * framebuffer - framebuffer instance.
 */
Image getFramebufferDepthStencilAttachment(Framebuffer framebuffer);
/*
 * Returns true if framebuffer is created by the window.
 * framebuffer - framebuffer instance.
 */
bool isFramebufferDefault(Framebuffer framebuffer);

/*
 * Set framebuffer attachments.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * size - framebuffer size in pixels.
 * useBeginClear - use begin function clear values.
 * colorAttachments - color attachment instance array or NULL.
 * colorAttachmentCount - color attachment count or 0.
 * depthStencilAttachment - depth/stencil attachment instance or NULL.
 */
MpgxResult setFramebufferAttachments(
	Framebuffer framebuffer,
	Vec2I size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment);

/*
 * Begin framebuffer render command recording.
 *
 * framebuffer - framebuffer instance.
 * clearValues - attachment clear values or NULL.
 * clearValueCount - clear values count or 0.
 */
void beginFramebufferRender(
	Framebuffer framebuffer,
	const FramebufferClear* clearValues,
	size_t clearValueCount);
/*
 * End framebuffer render command recording.
 * framebuffer - framebuffer instance.
 */
void endFramebufferRender(Framebuffer framebuffer);

/*
 * Clears framebuffer attachments.
 *
 * framebuffer - framebuffer instance.
 * clearAttachments - target clear attachment array.
 * clearValues - attachment clear value array.
 * clearValueCount - clear value count.
 */
void clearFramebuffer(
	Framebuffer framebuffer,
	const bool* clearAttachments,
	const FramebufferClear* clearValues,
	size_t clearValueCount);

/*
 * Create a new graphics pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * name - name string or NULL. (for debugging)
 * state - graphics pipeline state.
 * onBind - on graphics pipeline bind function or NULL.
 * onUniformsSet - on graphics pipeline uniforms set function or NULL.
 * onResize - on graphics pipeline resize function.
 * onDestroy - on graphics pipeline destroy function.
 * handle - graphics pipeline handle.
 * createData - Vulkan create data. (NULL in OpenGL)
 * shaders - shader instance array.
 * shaderCount - shader count.
 * graphicsPipeline - pointer to the graphics pipeline.
 */
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
/*
 * Destroys graphics pipeline instance.
 * pipeline - graphics pipeline instance or NULL.
 */
void destroyGraphicsPipeline(GraphicsPipeline pipeline);

/*
 * Returns graphics pipeline framebuffer instance.
 * pipeline - graphics pipeline instance.
 */
Framebuffer getGraphicsPipelineFramebuffer(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline name string. (for debugging)
 * pipeline - graphics pipeline instance.
 */
const char* getGraphicsPipelineName(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline state.
 * pipeline - graphics pipeline instance.
 */
const GraphicsPipelineState* getGraphicsPipelineState(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline on bind function.
 * pipeline - graphics pipeline instance.
 */
OnGraphicsPipelineBind getGraphicsPipelineOnBind(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline on uniforms set function.
 * pipeline - graphics pipeline instance.
 */
OnGraphicsPipelineUniformsSet getGraphicsPipelineOnUniformsSet(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline on resize function.
 * pipeline - graphics pipeline instance.
 */
OnGraphicsPipelineResize getGraphicsPipelineOnResize(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline on destroy function.
 * pipeline - graphics pipeline instance.
 */
OnGraphicsPipelineDestroy getGraphicsPipelineOnDestroy(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline handle.
 * pipeline - graphics pipeline instance.
 */
void* getGraphicsPipelineHandle(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline shader instance array.
 * pipeline - graphics pipeline instance.
 */
Shader* getGraphicsPipelineShaders(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline shader count.
 * pipeline - graphics pipeline instance.
 */
size_t getGraphicsPipelineShaderCount(GraphicsPipeline pipeline);
/*
 * Returns graphics pipeline window instance.
 * pipeline - graphics pipeline instance.
 */
Window getGraphicsPipelineWindow(GraphicsPipeline pipeline);

/*
 * Binds graphics pipeline. (rendering command)
 * pipeline - graphics pipeline instance.
 */
void bindGraphicsPipeline(GraphicsPipeline pipeline);

/*
 * Create a new graphics mesh instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * indexType - index type.
 * indexCount - index count or 0.
 * indexOffset - index offset or 0.
 * vertexBuffer - vertex buffer instance or NULL.
 * indexBuffer - index buffer instance or NULL.
 * graphicsMesh - pointer to the graphics mesh instance.
 */
MpgxResult createGraphicsMesh(
	Window window,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	GraphicsMesh* graphicsMesh);
/*
 * Destroys graphics mesh instance.
 * mesh - graphics mesh instance or NULL.
 */
void destroyGraphicsMesh(GraphicsMesh mesh);

/*
 * Returns graphics mesh window instance.
 * mesh - graphics mesh instance.
 */
Window getGraphicsMeshWindow(GraphicsMesh mesh);
/*
 * Returns graphics mesh index type.
 * mesh - graphics mesh instance.
 */
IndexType getGraphicsMeshIndexType(GraphicsMesh mesh);

/*
 * Returns graphics mesh index count.
 * mesh - graphics mesh instance.
 */
size_t getGraphicsMeshIndexCount(
	GraphicsMesh mesh);
/*
 * Sets graphics mesh index count.
 *
 * mesh - graphics mesh instance.
 * indexCount - index count or 0.
 */
void setGraphicsMeshIndexCount(
	GraphicsMesh mesh,
	size_t indexCount);

/*
 * Returns graphics mesh index offset.
 * mesh - graphics mesh instance.
 */
size_t getGraphicsMeshIndexOffset(
	GraphicsMesh mesh);
/*
 * Sets graphics mesh index offset.
 *
 * mesh - graphics mesh instance.
 * indexCount - index offset or 0.
 */
void setGraphicsMeshIndexOffset(
	GraphicsMesh mesh,
	size_t indexOffset);

/*
 * Returns graphics mesh vertex buffer instance.
 * mesh - graphics mesh instance.
 */
Buffer getGraphicsMeshVertexBuffer(
	GraphicsMesh mesh);
/*
 * Sets graphics mesh vertex buffer.
 *
 * mesh - graphics mesh instance.
 * vertexBuffer - vertex buffer instance or NULL.
 */
void setGraphicsMeshVertexBuffer(
	GraphicsMesh mesh,
	Buffer vertexBuffer);

/*
 * Returns graphics mesh index buffer instance.
 * mesh - graphics mesh instance.
 */
Buffer getGraphicsMeshIndexBuffer(
	GraphicsMesh mesh);
/*
 * Sets graphics mesh index buffer.
 *
 * mesh - graphics mesh instance.
 * indexType - index type.
 * indexCount - index count or 0.
 * indexOffset - index offset or 0.
 * indexBuffer - index buffer instance or NULL.
 */
void setGraphicsMeshIndexBuffer(
	GraphicsMesh mesh,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer indexBuffer);

/*
 * Draw graphics mesh. (rendering command)
 * Returns drawn index count.
 *
 * pipeline - graphics pipeline instance.
 * mesh - graphics mesh instance.
 */
size_t drawGraphicsMesh(
	GraphicsPipeline pipeline,
	GraphicsMesh mesh);

/*
 * Create a new compute pipeline instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * name - name string or NULL. (for debugging)
 * onBind - on compute pipeline bind function or NULL.
 * onDestroy - on compute pipeline destroy function.
 * handle - compute pipeline handle.
 * createData - Vulkan create data. (NULL in OpenGL)
 * shader - compute shader instance.
 * computePipeline - pointer to the compute pipeline instance.
 */
MpgxResult createComputePipeline(
	Window window,
	const char* name,
	OnComputePipelineBind onBind,
	OnComputePipelineDestroy onDestroy,
	void* handle,
	const void* createData,
	Shader shader,
	ComputePipeline* computePipeline);
/*
 * Destroys compute pipeline instance.
 * pipeline - compute pipeline instance or NULL.
 */
void destroyComputePipeline(ComputePipeline pipeline);

/*
 * Returns compute pipeline window instance.
 * pipeline - compute pipeline instance.
 */
Window getComputePipelineWindow(ComputePipeline pipeline);
/*
 * Returns compute pipeline name string. (for debugging)
 * pipeline - compute pipeline instance.
 */
const char* getComputePipelineName(ComputePipeline pipeline);
/*
 * Returns compute pipeline on bind function.
 * pipeline - compute pipeline instance.
 */
OnComputePipelineBind getComputePipelineOnBind(ComputePipeline pipeline);
/*
 * Returns compute pipeline on destroy function.
 * pipeline - compute pipeline instance.
 */
OnComputePipelineDestroy getComputePipelineOnDestroy(ComputePipeline pipeline);
/*
 * Returns compute pipeline shader instance.
 * pipeline - compute pipeline instance.
 */
Shader getComputePipelineShader(ComputePipeline pipeline);
/*
 * Returns compute pipeline handle.
 * pipeline - compute pipeline instance.
 */
void* getComputePipelineHandle(ComputePipeline pipeline);

/*
 * Bind compute pipeline. (rendering command)
 * pipeline - compute pipeline instance.
 */
void bindComputePipeline(ComputePipeline pipeline);

/*
 * Dispatches compute pipeline work. (rendering command)
 *
 * pipeline - compute pipeline instance.
 * groupCountX - group count along X-axis.
 * groupCountY - group count along Y-axis.
 * groupCountZ - group count along Z-axis.
 */
void dispatchComputePipeline(
	ComputePipeline pipeline,
	size_t groupCountX,
	size_t groupCountY,
	size_t groupCountZ);

// WARNING: RTX is not yet working!

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
void destroyRayTracingPipeline(RayTracingPipeline pipeline);

Window getRayTracingPipelineWindow(RayTracingPipeline pipeline);
const char* getRayTracingPipelineName(RayTracingPipeline pipeline);
OnRayTracingPipelineBind getRayTracingPipelineOnBind(RayTracingPipeline pipeline);
OnRayTracingPipelineDestroy getRayTracingPipelineOnDestroy(RayTracingPipeline pipeline);
void* getRayTracingPipelineHandle(RayTracingPipeline pipeline);
Shader* getRayTracingPipelineGenerationShaders(RayTracingPipeline pipeline);
size_t getRayTracingPipelineGenerationShaderCount(RayTracingPipeline pipeline);
Shader* getRayTracingPipelineMissShaders(RayTracingPipeline pipeline);
size_t getRayTracingPipelineMissShaderCount(RayTracingPipeline pipeline);
Shader* getRayTracingPipelineClosestHitShaders(RayTracingPipeline pipeline);
size_t getRayTracingPipelineClosestHitShaderCount(RayTracingPipeline pipeline);

void bindRayTracingPipeline(RayTracingPipeline pipeline);
void traceRayTracingPipeline(RayTracingPipeline pipeline);

MpgxResult createRayTracingMesh(
	Window window,
	size_t vertexStride,
	IndexType indexType,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	RayTracingMesh* rayTracingMesh);
void destroyRayTracingMesh(RayTracingMesh mesh);

Window getRayTracingMeshWindow(RayTracingMesh mesh);
size_t getRayTracingMeshVertexStride(RayTracingMesh mesh);
IndexType getRayTracingMeshIndexType(RayTracingMesh mesh);
Buffer getRayTracingMeshVertexBuffer(RayTracingMesh mesh);
Buffer getRayTracingMeshIndexBuffer(RayTracingMesh mesh);

MpgxResult createRayTracingScene(
	Window window,
	RayTracingMesh* meshes,
	size_t meshCount,
	RayTracingScene* rayTracingScene);
void destroyRayTracingScene(RayTracingScene scene);

Window getRayTracingSceneWindow(RayTracingScene scene);
RayTracingMesh* getRayTracingSceneMeshes(RayTracingScene scene);
size_t getRayTracingSceneMeshCount(RayTracingScene scene);
