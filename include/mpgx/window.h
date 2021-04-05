#pragma once
#include "cmmt/vector.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

// TODO:
// Add other enumerations

enum KEYBOARD_KEY
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
};

enum MOUSE_BUTTON
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
};

enum GRAPHICS_API
{
	VULKAN_GRAPHICS_API = 0,
	OPENGL_GRAPHICS_API = 1,
	OPENGL_ES_GRAPHICS_API = 2,
};

enum BUFFER_TYPE
{
	VERTEX_BUFFER_TYPE = 0,
	INDEX_BUFFER_TYPE = 1,
	UNIFORM_BUFFER_TYPE = 2,
};

enum DRAW_INDEX
{
	UINT8_DRAW_INDEX = 0,
	UINT32_DRAW_INDEX = 1,
	UINT16_DRAW_INDEX = 2,
};

enum IMAGE_TYPE
{
	IMAGE_2D_TYPE = 0,
	IMAGE_3D_TYPE = 1,
};

enum IMAGE_FORMAT
{
	R8G8B8A8_UNORM_IMAGE_FORMAT = 0,
	R8G8B8A8_SRGB_IMAGE_FORMAT = 1,
};

enum IMAGE_FILTER
{
	LINEAR_IMAGE_FILTER = 0,
	NEAREST_IMAGE_FILTER = 1,
};

enum IMAGE_WRAP
{
	REPEAT_IMAGE_WRAP = 0,
	MIRRORED_REPEAT_IMAGE_WRAP = 1,
	CLAMP_TO_EDGE_IMAGE_WRAP = 2,
};

enum SHADER_TYPE
{
	VERTEX_SHADER_TYPE = 0,
	FRAGMENT_SHADER_TYPE = 1,
	COMPUTE_SHADER_TYPE = 2,
};

enum DRAW_MODE
{
	POINTS_DRAW_MODE = 0,
	LINE_STRIP_DRAW_MODE = 1,
	LINE_LOOP_DRAW_MODE = 2,
	LINES_DRAW_MODE = 3,
	TRIANGLE_STRIP_DRAW_MODE = 4,
	TRIANGLE_FAN_DRAW_MODE = 5,
	TRIANGLES_DRAW_MODE = 6,
};

enum CULL_FACE
{
	BACK_ONLY_CULL_FACE = 0,
	FRONT_ONLY_CULL_FACE = 1,
	BACK_FRONT_CULL_FACE = 2,
};
enum FRONT_FACE
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
struct Vec2F getWindowContentScale(
	const struct Window* window);
struct Vec2I getWindowFramebufferSize(
	const struct Window* window);
bool getWindowKeyboardKey(
	const struct Window* window,
	int key);
bool getWindowMouseButton(
	const struct Window* window,
	int button);
const char* getWindowClipboard(
	const struct Window* window);

struct Vec2I getWindowSize(
	const struct Window* window);
void setWindowSize(
	struct Window* window,
	struct Vec2I size);

struct Vec2I getWindowPosition(
	const struct Window* window);
void setWindowPosition(
	struct Window* window,
	struct Vec2I position);

struct Vec2F getWindowCursorPosition(
	const struct Window* window);
void setWindowCursorPosition(
	struct Window* window,
	struct Vec2F position);

bool isWindowFocused(
	struct Window* window);
bool isWindowIconified(
	struct Window* window);
bool isWindowMaximized(
	struct Window* window);
bool isWindowVisible(
	struct Window* window);
bool isWindowHovered(
	struct Window* window);

void iconifyWindow(
	struct Window* window);
void maximizeWindow(
	struct Window* window);
void restoreWindow(
	struct Window* window);
void showWindow(
	struct Window* window);
void hideWindow(
	struct Window* window);
void focusWindow(
	struct Window* window);
void requestWindowAttention(
	struct Window* window);

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
const void* getBufferHandle(
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
struct Shader* createShaderFromFile(
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
