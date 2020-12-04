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

struct Window;

bool initializeGraphics();
void terminateGraphics();

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
