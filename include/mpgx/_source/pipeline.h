#pragma once
#include "mpgx/_source/opengl.h"
#include <assert.h>

#define NULL_UNIFORM_LOCATION -1

inline static GLint getGlUniformLocation(
	GLuint program,
	const GLchar* name)
{
	GLint uniformLocation = glGetUniformLocation(
		program,
		name);

#ifndef NDEBUG
	if (uniformLocation == NULL_UNIFORM_LOCATION)
	{
		fprintf(stderr,
			"Failed to get '%s' uniform location.\n",
			name);
	}
#endif

	return uniformLocation;
}
inline static GLuint getGlUniformBlockIndex(
	GLuint program,
	const GLchar* name)
{
	GLuint uniformBlockIndex = glGetUniformBlockIndex(
		program,
		name);

#ifndef NDEBUG
	if (uniformBlockIndex == GL_INVALID_INDEX)
	{
		fprintf(stderr,
			"Failed to get '%s' uniform block index.\n",
			name);
	}
#endif

	return uniformBlockIndex;
}

inline static GLuint createGlPipeline(
	Window window,
	Shader* _shaders,
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

		GLuint handle = (GLuint)(uintptr_t)
		getShaderHandle(_shaders[i]);

		glAttachShader(
			program,
			handle);
	}

	glLinkProgram(program);

	for (size_t i = 0; i < shaderCount; i++)
	{
		GLuint handle = (GLuint)(uintptr_t)
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
				(GLchar*)infoLog);

			fprintf(GL_INFO_LOG_OUT,
				"OpenGL program link error: %s\n",
				infoLog);

			free(infoLog);
		}

		assertOpenGL();

		glDeleteProgram(program);
		return GL_ZERO;
	}

	assertOpenGL();
	return program;
}
inline static void destroyGlPipeline(
	Window window,
	GLuint pipeline)
{
	assert(window != NULL);
	makeWindowContextCurrent(window);
	glDeleteProgram(pipeline);
	assertOpenGL();
}
