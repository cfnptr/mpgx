#pragma once
#include "mpgx/_source/opengl.h"

#if MPGX_SUPPORT_VULKAN
#include "mpgx/_source/vulkan.h"
#endif

#define OPENGL_SHADER_HEADER \
	"#version 330 core\n"
#define OPENGL_ES_SHADER_HEADER \
	"#version 300 es\n"         \
	"precision highp float;\n"  \
	"precision highp int;\n"

#include <string.h>

typedef struct _VkShader
{
	Window window;
	uint8_t type;
#if MPGX_SUPPORT_VULKAN
	VkShaderModule handle;
#endif
} _VkShader;
typedef struct _GlShader
{
	Window window;
	uint8_t type;
	GLuint handle;
} _GlShader;
union Shader
{
	_GlShader gl;
	_VkShader vk;
};

#if MPGX_SUPPORT_VULKAN
inline static Shader createVkShader(
	VkDevice device,
	Window window,
	uint8_t type,
	const void* code,
	size_t size)
{
	Shader shader = malloc(
		sizeof(union Shader));

	if (shader == NULL)
		return NULL;

	VkShaderModuleCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		NULL,
		0,
		size,
		code,
	};

	VkShaderModule handle;

	VkResult result = vkCreateShaderModule(
		device,
		&createInfo,
		NULL,
		&handle);

	if (result != VK_SUCCESS)
	{
		free(shader);
		return NULL;
	}

	shader->vk.window = window;
	shader->vk.type = type;
	shader->vk.handle = handle;
	return shader;
}
inline static void destroyVkShader(
	VkDevice device,
	Shader shader)
{
	vkDestroyShaderModule(
		device,
		shader->vk.handle,
		NULL);
	free(shader);
}
#endif

inline static bool getGlShaderType(
	uint8_t shaderType,
	GLenum* glShaderType)
{
	if (shaderType == VERTEX_SHADER_TYPE)
	{
		*glShaderType = GL_VERTEX_SHADER;
		return true;
	}
	else if (shaderType == FRAGMENT_SHADER_TYPE)
	{
		*glShaderType = GL_FRAGMENT_SHADER;
		return true;
	}
	else if (shaderType == COMPUTE_SHADER_TYPE)
	{
		*glShaderType = GL_COMPUTE_SHADER;
		return true;
	}
	else
	{
		return false;
	}
}
inline static Shader createGlShader(
	Window window,
	uint8_t type,
	const void* code,
	size_t size,
	GraphicsAPI api)
{
	Shader shader = malloc(
		sizeof(union Shader));

	if (shader == NULL)
		return NULL;

	GLenum glType;

	bool result = getGlShaderType(
		type,
		&glType);

	if (result == false)
	{
		free(shader);
		return NULL;
	}

	const char* sources[2];
	GLint lengths[2];

	if (api == OPENGL_GRAPHICS_API)
	{
		sources[0] = OPENGL_SHADER_HEADER;
		lengths[0] = (GLint)strlen(OPENGL_SHADER_HEADER);
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		sources[0] = OPENGL_ES_SHADER_HEADER;
		lengths[0] = (GLint)strlen(OPENGL_ES_SHADER_HEADER);
	}
	else
	{
		abort();
	}

	sources[1] = (const char*)code;
	lengths[1] = (GLint)size;

	makeWindowContextCurrent(window);

	GLuint handle = glCreateShader(glType);

	glShaderSource(
		handle,
		2,
		sources,
		lengths);

	glCompileShader(handle);

	GLint compileStatus;

	glGetShaderiv(
		handle,
		GL_COMPILE_STATUS,
		&compileStatus);

	if (compileStatus == GL_FALSE)
	{
		GLint length = 0;

		glGetShaderiv(
			handle,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				glDeleteShader(handle);
				free(shader);
				return NULL;
			}

			glGetShaderInfoLog(
				handle,
				length,
				&length,
				(GLchar*)infoLog);

			const char* typeString;

			if (type == VERTEX_SHADER_TYPE)
				typeString = "vertex";
			else if (type == FRAGMENT_SHADER_TYPE)
				typeString = "fragment";
			else
				typeString = "compute";

			printf("OpenGL %s shader compile error:\n%s",
				typeString,
				infoLog);
			free(infoLog);
		}

		assertOpenGL();

		glDeleteShader(handle);
		free(shader);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		glDeleteShader(handle);
		free(shader);
		return NULL;
	}

	shader->gl.window = window;
	shader->gl.type = type;
	shader->gl.handle = handle;
	return shader;
}
inline static void destroyGlShader(
	Shader shader)
{
	makeWindowContextCurrent(
		shader->gl.window);

	glDeleteShader(
		shader->gl.handle);
	assertOpenGL();

	free(shader);
}
