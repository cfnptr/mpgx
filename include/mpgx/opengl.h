#pragma once
#include "mpgx/window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <assert.h>

#define OPENGL_SHADER_HEADER "#version 330 core\n\n#define highp \n#define mediump \n#define lowp \n"
#define OPENGL_ES_SHADER_HEADER "#version 300 es\n"

#define OPENGL_COLOR_VERTEX_SHADER                 \
"layout(location = 0) in highp vec3 v_Position;\n  \
uniform highp mat4 u_MVP;\n                        \
                                                   \
void main()\n                                      \
{\n                                                \
	gl_Position = u_MVP * vec4(v_Position, 1.0);\n \
}\n"
#define OPENGL_COLOR_FRAGMENT_SHADER \
"out highp vec4 o_Color;\n           \
uniform highp vec4 u_Color;\n        \
                                     \
void main()\n                        \
{\n                                  \
	o_Color = u_Color;\n             \
}\n"

inline static bool getGlDrawMode(
	enum DrawMode drawMode,
	GLenum* glDrawMode)
{
	assert(glDrawMode != NULL);

	switch (drawMode)
	{
	default:
		return false;
	case POINTS_DRAW_MODE:
		*glDrawMode = GL_POINTS;
		return true;
	case LINE_STRIP_DRAW_MODE:
		*glDrawMode = GL_LINE_STRIP;
		return true;
	case LINE_LOOP_DRAW_MODE:
		*glDrawMode = GL_LINE_LOOP;
		return true;
	case LINES_DRAW_MODE:
		*glDrawMode = GL_LINES;
		return true;
	case TRIANGLE_STRIP_DRAW_MODE:
		*glDrawMode = GL_TRIANGLE_STRIP;
		return true;
	case TRIANGLE_FAN_DRAW_MODE:
		*glDrawMode = GL_TRIANGLE_FAN;
		return true;
	case TRIANGLES_DRAW_MODE:
		*glDrawMode = GL_TRIANGLES;
		return true;
	}
}
inline static GLuint createGlShader(
	GLenum stage,
	const char* source,
	bool gles)
{
	GLuint shader = glCreateShader(stage);

	const char* sources[2];

	if (gles == false)
		sources[0] = OPENGL_SHADER_HEADER;
	else
		sources[0] = OPENGL_ES_SHADER_HEADER;

	sources[1] = source;

	glShaderSource(
		shader,
		2,
		sources,
		NULL);

	glCompileShader(shader);

	GLint result;

	glGetShaderiv(
		shader,
		GL_COMPILE_STATUS,
		&result);

	if (result == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetShaderiv(
			shader,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char infoLog[length];

			glGetShaderInfoLog(
				shader,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		glDeleteShader(shader);
		return GL_ZERO;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	return shader;
}
inline static GLuint createGlPipeline(
	const GLenum* shaderStages,
	const char** shaderSources,
	size_t shaderCount,
	bool gles)
{
	assert(shaderStages != NULL);
	assert(shaderSources != NULL);
	assert(shaderCount != 0);

	GLuint program = glCreateProgram();

	GLuint shaders[shaderCount];

	for (size_t i = 0; i < shaderCount; i++)
	{
		shaders[i] = createGlShader(
			shaderStages[i],
			shaderSources[i],
			gles);
	}

	for (size_t i = 0; i < shaderCount; i++)
	{
		glAttachShader(
			program,
			shaders[i]);
	}

	glLinkProgram(program);

	for (size_t i = 0; i < shaderCount; i++)
	{
		glDetachShader(
			program,
			shaders[i]);
	}

	for (size_t i = 0; i < shaderCount; i++)
		glDeleteShader(shaders[i]);

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
			char infoLog[length];

			glGetProgramInfoLog(
				program,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		glDeleteProgram(program);
		return GL_ZERO;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	return program;
}
