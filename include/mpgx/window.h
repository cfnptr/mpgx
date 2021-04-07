#pragma once
#include "cmmt/vector.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

// TODO: Add other enumerations

typedef enum KEYBOARD_KEY
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
} KEYBOARD_KEY;

typedef enum MOUSE_BUTTON
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
} MOUSE_BUTTON;

typedef enum CURSOR_MODE
{
	DEFAULT_CURSOR_MODE,
	HIDDEN_CURSOR_MODE,
	LOCKED_CURSOR_MODE,
	CURSOR_MODE_COUNT,
} CURSOR_MODE;

typedef enum GRAPHICS_API
{
	VULKAN_GRAPHICS_API,
	OPENGL_GRAPHICS_API,
	OPENGL_ES_GRAPHICS_API,
	GRAPHICS_API_COUNT,
} GRAPHICS_API;

typedef enum BUFFER_TYPE
{
	VERTEX_BUFFER_TYPE,
	INDEX_BUFFER_TYPE,
	UNIFORM_BUFFER_TYPE,
	BUFFER_TYPE_COUNT,
} BUFFER_TYPE;

typedef enum DRAW_INDEX
{
	UINT16_DRAW_INDEX,
	UINT32_DRAW_INDEX,
	DRAW_INDEX_COUNT,
} DRAW_INDEX;

typedef enum IMAGE_TYPE
{
	IMAGE_2D_TYPE,
	IMAGE_3D_TYPE,
	IMAGE_TYPE_COUNT,
} IMAGE_TYPE;

typedef enum IMAGE_FORMAT
{
	R8G8B8A8_UNORM_IMAGE_FORMAT,
	R8G8B8A8_SRGB_IMAGE_FORMAT,
	IMAGE_FORMAT_COUNT,
} IMAGE_FORMAT;

typedef enum IMAGE_FILTER
{
	LINEAR_IMAGE_FILTER,
	NEAREST_IMAGE_FILTER,
	IMAGE_FILTER_COUNT,
} IMAGE_FILTER;

typedef enum IMAGE_WRAP
{
	REPEAT_IMAGE_WRAP,
	MIRRORED_REPEAT_IMAGE_WRAP,
	CLAMP_TO_EDGE_IMAGE_WRAP,
	IMAGE_WRAP_COUNT,
} IMAGE_WRAP;

typedef enum SHADER_TYPE
{
	VERTEX_SHADER_TYPE,
	FRAGMENT_SHADER_TYPE,
	COMPUTE_SHADER_TYPE,
	SHADER_TYPE_COUNT,
} SHADER_TYPE;

typedef enum DRAW_MODE
{
	POINTS_DRAW_MODE,
	LINE_STRIP_DRAW_MODE,
	LINE_LOOP_DRAW_MODE,
	LINES_DRAW_MODE,
	TRIANGLE_STRIP_DRAW_MODE,
	TRIANGLE_FAN_DRAW_MODE,
	TRIANGLES_DRAW_MODE,
	DRAW_MODE_COUNT,
} DRAW_MODE;

typedef enum CULL_FACE
{
	BACK_ONLY_CULL_FACE,
	FRONT_ONLY_CULL_FACE,
	BACK_FRONT_CULL_FACE,
	CULL_FACE_COUNT,
} CULL_FACE;
typedef enum FRONT_FACE
{
	CLOCKWISE_FRONT_FACE,
	COUNTERCLOCKWISE_FRONT_FACE,
	FRONT_FACE_COUNT,
} FRONT_FACE;

typedef struct Window Window;
typedef union Buffer Buffer;
typedef union Mesh Mesh;
typedef union Image Image;
typedef union Framebuffer Framebuffer;
typedef union Shader Shader;
typedef struct Pipeline Pipeline;
// TODO:
//union Query
//union Sampler?

typedef void(*UpdateWindow)(void* argument);

typedef void(*DestroyPipeline)(
	Window* window,
	void* pipeline);
typedef void(*BindPipelineCommand)(
	Pipeline* pipeline);
typedef void(*SetUniformsCommand)(
	Pipeline* pipeline);

bool initializeGraphics();
void terminateGraphics();
bool isGraphicsInitialized();

void* getFtLibrary();

Window* createWindow(
	uint8_t api,
	size_t width,
	size_t height,
	const char* title,
	UpdateWindow updateFunction,
	void* updateArgument);
Window* createAnyWindow(
	size_t width,
	size_t height,
	const char* title,
	UpdateWindow updateFunction,
	void* updateArgument);
void destroyWindow(Window* window);

uint8_t getWindowGraphicsAPI(const Window* window);
size_t getWindowMaxImageSize(const Window* window);
double getWindowUpdateTime(const Window* window);
double getWindowDeltaTime(const Window* window);
Vector2F getWindowContentScale(const Window* window);
Vector2I getWindowFramebufferSize(const Window* window);
const char* getWindowClipboard(const Window* window);

bool getWindowKeyboardKey(
	const Window* window,
	int key);
bool getWindowMouseButton(
	const Window* window,
	int button);

Vector2I getWindowSize(
	const Window* window);
void setWindowSize(
	Window* window,
	Vector2I size);

Vector2I getWindowPosition(
	const Window* window);
void setWindowPosition(
	Window* window,
	Vector2I position);

Vector2F getWindowCursorPosition(
	const Window* window);
void setWindowCursorPosition(
	Window* window,
	Vector2F position);

uint8_t getWindowCursorMode(
	const Window* window);
void setWindowCursorMode(
	Window* window,
	uint8_t cursorMode);

bool isWindowFocused(Window* window);
bool isWindowIconified(Window* window);
bool isWindowMaximized(Window* window);
bool isWindowVisible(Window* window);
bool isWindowHovered(Window* window);

void iconifyWindow(Window* window);
void maximizeWindow(Window* window);
void restoreWindow(Window* window);
void showWindow(Window* window);
void hideWindow(Window* window);
void focusWindow(Window* window);
void requestWindowAttention(Window* window);

void makeWindowContextCurrent(Window* window);
void updateWindow(Window* window);

void beginCommandRecord(Window* window);
void endCommandRecord(Window* window);

Buffer* createBuffer(
	Window* window,
	uint8_t type,
	const void* data,
	size_t size,
	bool constant);
void destroyBuffer(Buffer* buffer);

Window* getBufferWindow(const Buffer* buffer);
uint8_t getBufferType(const Buffer* buffer);
size_t getBufferSize(const Buffer* buffer);
bool isBufferConstant(const Buffer* buffer);
const void* getBufferHandle(const Buffer* buffer);

void setBufferData(
	Buffer* buffer,
	const void* data,
	size_t size,
	size_t offset);

Mesh* createMesh(
	Window* window,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer* vertexBuffer,
	Buffer* indexBuffer);
void destroyMesh(Mesh* mesh);

Window* getMeshWindow(const Mesh* mesh);
uint8_t getMeshDrawIndex(const Mesh* mesh);

size_t getMeshIndexCount(
	const Mesh* mesh);
void setMeshIndexCount(
	Mesh* mesh,
	size_t indexCount);

size_t getMeshIndexOffset(
	const Mesh* mesh);
void setMeshIndexOffset(
	Mesh* mesh,
	size_t indexOffset);

Buffer* getMeshVertexBuffer(
	const Mesh* mesh);
void setMeshVertexBuffer(
	Mesh* mesh,
	Buffer* vertexBuffer);

Buffer* getMeshIndexBuffer(
	const Mesh* mesh);
void setMeshIndexBuffer(
	Mesh* mesh,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer* indexBuffer);

void getMeshBuffers(
	const Mesh* mesh,
	Buffer** vertexBuffer,
	Buffer** indexBuffer);
void setMeshBuffers(
	Mesh* mesh,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer* vertexBuffer,
	Buffer* indexBuffer);

void drawMeshCommand(
	Mesh* mesh,
	Pipeline* pipeline);

Image* createImage(
	Window* window,
	uint8_t type,
	uint8_t format,
	size_t width,
	size_t height,
	size_t depth,
	const void* pixels,
	bool useMipmap);
void destroyImage(Image* image);

void setImageData(
	Image* image,
	const void* data,
	size_t width,
	size_t height,
	size_t depth,
	size_t widthOffset,
	size_t heightOffset,
	size_t depthOffset,
	size_t mipmapLevel);
void generateMipmaps(Image* image);

Window* getImageWindow(const Image* image);
uint8_t getImageType(const Image* image);
uint8_t getImageFormat(const Image* image);
size_t getImageWidth(const Image* image);
size_t getImageHeight(const Image* image);
size_t getImageDepth(const Image* image);
bool isImageUseMipmapping(const Image* image);
const void* getImageHandle(const Image* image);

Shader* createShader(
	Window* window,
	uint8_t type,
	const void* code,
	size_t size);
Shader* createShaderFromFile(
	Window* window,
	uint8_t type,
	const char* filePath);
void destroyShader(Shader* shader);

Window* getShaderWindow(const Shader* shader);
uint8_t getShaderType(const Shader* shader);
const void* getShaderHandle(const Shader* shader);

Pipeline* createPipeline(
	Window* window,
	uint8_t drawMode,
	DestroyPipeline destroyFunction,
	BindPipelineCommand bindFunction,
	SetUniformsCommand setUniformsFunction,
	void* handle);
void destroyPipeline(Pipeline* pipeline);

Window* getPipelineWindow(
	const Pipeline* pipeline);
DestroyPipeline getPipelineDestroyFunction(
	const Pipeline* pipeline);
BindPipelineCommand getPipelineBindFunction(
	const Pipeline* pipeline);
SetUniformsCommand getPipelineSetUniformsFunction(
	const Pipeline* pipeline);
void* getPipelineHandle(
	const Pipeline* pipeline);

uint8_t getPipelineDrawMode(
	const Pipeline* pipeline);
void setPipelineDrawMode(
	Pipeline* pipeline,
	uint8_t drawMode);

void bindPipelineCommand(Pipeline* pipeline);
