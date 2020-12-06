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

enum ShaderStage
{
	VERTEX_SHADER_STAGE,
	FRAGMENT_SHADER_STAGE,
};

struct Window;
struct Shader;

bool initializeGraphics();
void terminateGraphics();

struct Window* createWindow(
	enum GraphicsAPI api,
	size_t width,
	size_t height,
	const char* title);
void destroyWindow(
	struct Window* window);

bool getWindowUpdateTime(
	struct Window* window,
	double* time);
bool getWindowDeltaTime(
	struct Window* window,
	double* time);

bool startWindowUpdate(
	struct Window* window);

struct Shader* createShader(
	struct Window* window,
	enum ShaderStage stage,
	const char* code);
void destroyShader(
	struct Shader* shader);
