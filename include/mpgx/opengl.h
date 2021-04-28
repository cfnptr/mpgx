#pragma once

#if MPGX_VULKAN_SUPPORT
#define GLFW_INCLUDE_VULKAN
#endif

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

inline static void assertOpenGL()
{
#ifndef NDEBUG
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
#endif
}

inline static GLuint createGlPipeline(
	Window* window,
	Shader** _shaders,
	size_t shaderCount)
{
	assert(window != NULL);
	assert(_shaders != NULL);
	assert(shaderCount != 0);

	makeWindowContextCurrent(window);

	GLuint program = glCreateProgram();

	for (size_t i = 0; i < shaderCount; i++)
	{
		assert(getShaderWindow(_shaders[i]) == window);

		GLuint handle = *(const GLuint*)
			getShaderHandle(_shaders[i]);

		glAttachShader(
			program,
			handle);
	}

	glLinkProgram(program);

	for (size_t i = 0; i < shaderCount; i++)
	{
		GLuint handle = *(const GLuint*)
			getShaderHandle(_shaders[i]);

		glDetachShader(
			program,
			handle);
	}

	GLint result;

	glGetProgramiv(
		program,
		GL_LINK_STATUS,
		&result);

	if (result == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetProgramiv(
			program,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				glDeleteProgram(program);
				return GL_ZERO;
			}

			glGetProgramInfoLog(
				program,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
			free(infoLog);
		}
#endif
		assertOpenGL();

		glDeleteProgram(program);
		return GL_ZERO;
	}

	assertOpenGL();
	return program;
}
inline static void destroyGlPipeline(
	Window* window,
	GLuint pipeline)
{
	assert(window != NULL);
	makeWindowContextCurrent(window);
	glDeleteProgram(pipeline);
	assertOpenGL();
}

inline static GLenum getGlImageFilter(
	uint8_t imageFilter,
	uint8_t mipmapFilter,
	bool useMipmapping)
{
	if (imageFilter == NEAREST_IMAGE_FILTER)
	{
		if (useMipmapping == true)
		{
			if(mipmapFilter == NEAREST_IMAGE_FILTER)
				return GL_NEAREST_MIPMAP_NEAREST;
			else
				return GL_NEAREST_MIPMAP_LINEAR;
		}
		else
		{
			return GL_NEAREST;
		}
	}
	else
	{
		if (useMipmapping == true)
		{
			if (mipmapFilter == NEAREST_IMAGE_FILTER)
				return GL_LINEAR_MIPMAP_NEAREST;
			else
				return GL_LINEAR_MIPMAP_LINEAR;
		}
		else
		{
			return GL_LINEAR;
		}
	}
}
inline static GLenum getGlImageWrap(uint8_t wrap)
{
	if (wrap == CLAMP_TO_EDGE_IMAGE_WRAP)
		return GL_CLAMP_TO_EDGE;
	else if (wrap == MIRRORED_REPEAT_IMAGE_WRAP)
		return GL_MIRRORED_REPEAT;
	else
		return GL_REPEAT;
}
inline static GLenum getGlImageCompare(uint8_t compare)
{
	switch (compare)
	{
	default:
		return GL_LESS;
	case LESS_EQUAL_IMAGE_COMPARE:
		return GL_LEQUAL;
	case GREATER_EQUAL_IMAGE_COMPARE:
		return GL_GEQUAL;
	case GREATER_IMAGE_COMPARE:
		return GL_GREATER;
	case EQUAL_IMAGE_COMPARE:
		return GL_EQUAL;
	case NOT_EQUAL_IMAGE_COMPARE:
		return GL_NOTEQUAL;
	case ALWAYS_IMAGE_COMPARE:
		return GL_ALWAYS;
	case NEVER_IMAGE_COMPARE:
		return GL_NEVER;
	}
}
