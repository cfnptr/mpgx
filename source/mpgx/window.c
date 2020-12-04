#include "mpgx/window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <assert.h>
#include <stdbool.h>

// TMP
#include <stdio.h>

struct Window
{
	enum GraphicsAPI api;
	struct GLFWwindow* handle;
	double updateTime;
	double deltaTime;
};

static bool graphicsInitialized = false;

bool initializeGraphics()
{
	if (graphicsInitialized == true)
		return false;

	int result = glfwInit();

	if(result == GLFW_FALSE)
		return false;

	graphicsInitialized = true;
	return true;
}
void terminateGraphics()
{
	if (graphicsInitialized == false)
		return;

	glfwTerminate();
	graphicsInitialized = false;
}

struct Window* createWindow(
	enum GraphicsAPI api,
	size_t width,
	size_t height,
	const char* title)
{
	assert(width > 0);
	assert(height > 0);
	assert(title != NULL);

	if (graphicsInitialized == false)
		return NULL;

	struct Window* window =
		malloc(sizeof(struct Window));

	if (window == NULL)
		return NULL;

	window->api = api;
	window->updateTime = 0.0;
	window->deltaTime = 0.0;

	glfwDefaultWindowHints();

	if (window->api == VULKAN_GRAPHICS_API)
	{
		// TODO:
		return NULL;
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_OPENGL_API);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MAJOR,
			3);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MINOR,
			3);
		glfwWindowHint(
			GLFW_OPENGL_FORWARD_COMPAT,
			GLFW_TRUE);
		glfwWindowHint(
			GLFW_OPENGL_PROFILE,
			GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_TRUE);
#else
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_FALSE);
#endif
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		glfwWindowHint(
			GLFW_CLIENT_API,
			GLFW_OPENGL_ES_API);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MAJOR,
			3);
		glfwWindowHint(
			GLFW_CONTEXT_VERSION_MINOR,
			0);

#ifndef NDEBUG
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_TRUE);
#else
		glfwWindowHint(
			GLFW_OPENGL_DEBUG_CONTEXT,
			GLFW_FALSE);
#endif
	}
	else
	{
		abort();
	}

	struct GLFWwindow* handle = glfwCreateWindow(
		width,
		height,
		title,
		NULL,
		NULL);

	if (handle == NULL)
	{
		free(window);
		return NULL;
	}

	glfwMakeContextCurrent(handle);

	if (gladLoadGL() == 0)
	{
		glfwDestroyWindow(handle);
		free(window);
		return NULL;
	}

	window->handle = handle;
	return window;
}
void destroyWindow(
	struct Window* window)
{
	if (window != NULL)
	{
		glfwDestroyWindow(
			window->handle);
	}

	free(window);
}

double getWindowUpdateTime(
	struct Window* window)
{
	assert(window != NULL);
	return window->updateTime;
}
double getWindowDeltaTime(
	struct Window* window)
{
	assert(window != NULL);
	return window->deltaTime;
}

void startWindowUpdate(
	struct Window* window)
{
	assert(window != NULL);

	struct GLFWwindow* handle =
		window->handle;

	if (window->api == VULKAN_GRAPHICS_API)
	{
		//TODO:
	}
	else if (window->api == OPENGL_GRAPHICS_API ||
		window->api == OPENGL_ES_GRAPHICS_API)
	{
		while (glfwWindowShouldClose(handle) == GLFW_FALSE)
		{
			glfwPollEvents();

			double time = glfwGetTime();
			window->deltaTime = time - window->updateTime;
			window->updateTime = time;

			// TODO: render

			glfwSwapBuffers(handle);
		}
	}
	else
	{
		abort();
	}
}
