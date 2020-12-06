#include "mpgx/window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdbool.h>

struct Window
{
	enum GraphicsAPI api;
	struct GLFWwindow* handle;
	double updateTime;
	double deltaTime;
};

static bool graphicsInitialized = false;

void glfwErrorCallback(
	int error,
	const char* description)
{
	fprintf(
		stderr,
		"ERROR: %s\n",
		description);
}

bool initializeGraphics()
{
	if (graphicsInitialized == true)
		return false;

	int result = glfwInit();

	if(result == GLFW_FALSE)
		return false;

	glfwSetErrorCallback(
		glfwErrorCallback);

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
	if (width == 0 ||
		height == 0 ||
		graphicsInitialized == false)
	{
		return NULL;
	}

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
		free(window);
		return NULL;
	}

	window->handle = glfwCreateWindow(
		(int)width,
		(int)height,
		title,
		NULL,
		NULL);

	if (window->handle == NULL)
	{
		free(window);
		return NULL;
	}

	glfwMakeContextCurrent(window->handle);

	if (gladLoadGL() == 0)
	{
		glfwDestroyWindow(window->handle);
		free(window);
		return NULL;
	}

	return window;
}
void destroyWindow(
	struct Window* window)
{
	if (window == NULL)
        return;
    
    glfwDestroyWindow(window->handle);
	free(window);
}

bool getWindowUpdateTime(
	struct Window* window,
	double* time)
{
	if (window == NULL ||
		time == NULL)
	{
		return NULL;
	}

	*time = window->updateTime;
	return true;
}
bool getWindowDeltaTime(
	struct Window* window,
	double* time)
{
	if (window == NULL ||
		time == NULL)
	{
		return NULL;
	}

	*time = window->deltaTime;
	return true;
}

bool startWindowUpdate(
	struct Window* window)
{
	if (window == NULL)
		return false;

	struct GLFWwindow* handle =
		window->handle;

	if (window->api == VULKAN_GRAPHICS_API)
	{
		//TODO:
		return false;
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
		return false;
	}
}

struct Shader* createShader(
	struct Window* window,
	enum ShaderStage stage,
	const char* code)
{
	if (window == NULL ||
		code == NULL)
	{
		return NULL;
	}

	if (window->api == VULKAN_GRAPHICS_API)
	{
		// TODO:
		return NULL;
	}
	else if (window->api == OPENGL_GRAPHICS_API)
	{
		return NULL;
	}
	else
	{
		return NULL;
	}
}
void destroyShader(
	struct Shader* shader)
{

}
